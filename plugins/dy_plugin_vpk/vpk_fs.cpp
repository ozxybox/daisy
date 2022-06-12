#include "vpk_fs.h"
#include "vpk_format.h"
#include <assert.h>
#include <X9PClient.h>



// VPKFS Helpers

uint32_t vpkfs_iounit(xhnd hnd)
{
	// Enough space for a maxmsg read
	return hnd->auth->client->m_maxmessagesize - sizeof(Tread_t);
}

qid_t vpkfs_nodeqid(vpkfs_node* node)
{
	qid_t qid;
	qid.path = 0;
	qid.type = node->is_dir ? X9P_QT_DIR : X9P_QT_FILE;
	qid.version = 0;
	return qid;
}

size_t vpkfs_stat(vpkfs_direntry* de, stat_t* stats, uint32_t max)
{
	vpkfs_node* node = de->node;

	// Filler text for perms stuff
	xstr_t blank = (xstr_t)"\x01\x00_";


	// Total size
	size_t sz = sizeof(stat_t) + de->name->size() + 3 * blank->size();
	
	// If we weren't given somewhere to copy into, just give back the space we took up
	if (!stats) return sz;
	assert(sz <= max);


	stats->size = sz;

	// We should really just zero everything that might go over the network for security reasons
	memset(&stats->_unused[0], 0, 6);


	stats->qid = vpkfs_nodeqid(node);

	dirmode_t mode = X9P_DM_READ | X9P_DM_EXEC;
	if (node->is_dir)
		mode |= X9P_DM_DIR;
	stats->mode = mode;


	// Yes, this file is totally from January 1, 1970
	stats->atime = 0;
	stats->mtime = 0;


	vpkfs_file* file = reinterpret_cast<vpkfs_file*>(node);
	stats->length = node->is_dir ? 0 : file->data_length + file->preload_length;

	xstrcpy(stats->name(), de->name);
	xstrcpy(stats->uid(), blank);
	xstrcpy(stats->gid(), blank);
	xstrcpy(stats->muid(), blank);

	return sz;
}



// VPK File System

CVPKFileSystem::CVPKFileSystem(const char* vpk_path) : CVPKArchivist(vpk_path)
{
	//CVPKArchivist aa(vpk_path);
}


vpkfs_deid CVPKFileSystem::GetEntryID(vpkfs_direntry* node)
{
	uintptr_t i = node - m_direntries;
	if (i >= m_direntrycount)
		return XHID_UNINITIALIZED;
	return i;
}
vpkfs_direntry* CVPKFileSystem::GetIDEntry(vpkfs_deid id)
{
	if (id < m_direntrycount)
		return &m_direntries[id];
	return 0;
}


void CVPKFileSystem::TagHND(xhnd hnd, vpkfs_direntry* de)
{
	hnd->fs = this;
	hnd->id = GetEntryID(de);
}
vpkfs_direntry* CVPKFileSystem::HNDValue(xhnd hnd)
{
	if (hnd->fs != this)
		return 0;
	return GetIDEntry(hnd->id);
}




vpkfs_direntry* CVPKFileSystem::GetNodeChild(vpkfs_node* node, xstr_t name)
{
	if (!node->is_dir)
		return 0;

	vpkfs_dir* wd = reinterpret_cast<vpkfs_dir*>(node);

	auto f = wd->children.find(name);
	if (f == wd->children.end())
		return 0;
	
	return f->second;
}



void CVPKFileSystem::Tattach(xhnd hnd, xhnd ahnd, xstr_t username, xstr_t accesstree, Rattach_fn callback)
{
	if (HNDValue(hnd))
	{
		callback(XERR_FID_USED, 0);
		return;
	}

	vpkfs_direntry* de = &m_direntries[0];

	TagHND(hnd, de);

	qid_t q = vpkfs_nodeqid(de->node);
	callback(0, &q);
}

void CVPKFileSystem::Twalk(xhnd hnd, xhnd newhnd, uint16_t nwname, xstr_t wname, Rwalk_fn callback)
{
	if (nwname > X9P_TWALK_MAXELEM)
	{
		callback(XERR_WALK_TOO_FAR, 0, 0);
		return;
	}

	// Does the first handle exist?
	vpkfs_direntry* de = HNDValue(hnd);
	if (!de)
	{
		callback(XERR_XHND_INVALID, 0, 0);
		return;
	}


	// Check if we can use the second handle
	if (hnd->id != newhnd->id)
	{
		// The id's are different 
		if (newhnd->id < m_direntrycount)
		{
			// id already in use
			callback(XERR_FID_USED, 0, 0);
			return;
		}
		// Not in use, free to use
	}
	// If they're the same, we're just overriding the first one 



	qid_t wqid[X9P_TWALK_MAXELEM];

	int i = 0;
	uint16_t n = nwname;
	for (xstr_t name = wname; i < n; name = xstrnext(name))
	{

		if (de->node->is_dir)
		{

			if (name->len == 1 && *name->str() == '.')
			{
				// Accessing '.'? That's against protocol!
				break;
			}
			else if (name->len == 2 && *reinterpret_cast<short*>(name->str()) == *reinterpret_cast<short*>(".."))
			{
				// Accessing '..'? Go back a directory.
				de = de->parent;
			}
			else
			{
				vpkfs_direntry* f = GetNodeChild(de->node, name);
				if (!f)
				{
					// Does not exist!
					break;
				}

				// Update our pos
				de = f;
			}

			// Add the qid to the arr
			wqid[i] = vpkfs_nodeqid(de->node);
		}
		else
		{
			// Cant walk into a file!
			break;
		}

		i++;
	}

	// We can only have a nwqid of 0 IF nwname was 0 as well

#if !X9P_V9FS_COMPAT
	if (i == 0 && nwname != 0)
		callback(XERR_FILE_DNE, 0, 0);
#endif

	// Tag the new hnd
	TagHND(newhnd, de);

	// Respond
	callback(0, i, &wqid[0]);

}

void CVPKFileSystem::Topen(xhnd hnd, openmode_t mode, Ropen_fn callback)
{
	vpkfs_direntry* de = HNDValue(hnd);
	if (!de)
	{
		callback(XERR_XHND_INVALID, 0, 0);
		return;
	}

	qid_t qid = vpkfs_nodeqid(de->node);
	uint32_t iounit = vpkfs_iounit(hnd);
	callback(0, &qid, iounit);
}

void CVPKFileSystem::Tread(xhnd hnd, uint64_t offset, uint32_t count, Rread_fn callback)
{
	vpkfs_direntry* de = HNDValue(hnd);
	if (!de)
	{
		callback(XERR_XHND_INVALID, 0, 0);
		return;
	}

	if (de->node->is_dir)
	{
		// Reading from a directory
		vpkfs_dir* nd = reinterpret_cast<vpkfs_dir*>(de->node);

		// Trek forward until our offset matches the direntry
		vpkfs_direntry* cd = 0;
		uint64_t o = 0;
		size_t sz = 0;
		for (auto& v : nd->children)
		{
			if (o > offset)
				break;
			
			cd = v.second;
			sz = vpkfs_stat(cd, 0, 0);
			o += sz;
		}

		//FIXME: Check if we're actually reading full blocks, or if we're on boundaries, or something 

		// Do we still have anything more to respond with?
		if (offset >= o)
		{
			// No more!
			callback(0, 0, 0);
			return;
		}
		
		assert(o - sz == offset);


		// Respond to the read with the child's stat
		stat_t* stat = (stat_t*)calloc(sz, 1);
		vpkfs_stat(cd, stat, sz);

		callback(0, sz, (void*)stat);

		free(stat);
	}
	else
	{
		// Reading from a file
		vpkfs_file* nf = reinterpret_cast<vpkfs_file*>(de->node);
		uint32_t fsz = nf->data_length + nf->preload_length;


		if (offset >= fsz)
		{
			// Cannot read past the file's length
			callback(0, 0, 0);
			return;
		}

		// Cap the read count off
		uint32_t rc = count + offset;
		if (rc > fsz)
			rc = fsz;
		rc -= offset;


		uint8_t* data = (uint8_t*)calloc(rc, 1);
		
		size_t sz = 0;

		//sz = node->Read(offset, sz, data);

		callback(0, sz, data);

		free(data);

	}
}

void CVPKFileSystem::Tstat(xhnd hnd, Rstat_fn callback)
{
	vpkfs_direntry* de = HNDValue(hnd);
	if (!de)
	{
		callback(XERR_XHND_INVALID, 0);
		return;
	}

	size_t sz = vpkfs_stat(de, 0, 0);
	stat_t* stat = (stat_t*)calloc(sz, 1);
	vpkfs_stat(de, stat, sz);

	callback(0, stat);

	free(stat);
}




// We don't do real opens, so our hnds can correlate directly to files
// So clunks don't do much for now
void CVPKFileSystem::Tclunk(xhnd hnd, Rclunk_fn callback) { callback(0); }

// Any modifying calls are stubbed off
void CVPKFileSystem::Tcreate(xhnd hnd, xstr_t name, dirmode_t perm, openmode_t mode, Rcreate_fn callback) { callback("Cannot create new files within a VPK!", 0, 0); }
void CVPKFileSystem::Twrite(xhnd hnd, uint64_t offset, uint32_t count, void* data, Rwrite_fn callback)    { callback("Cannot write to files within a VPK!", 0);      }
void CVPKFileSystem::Tremove(xhnd hnd, Rremove_fn callback)                                               { callback("Cannot remove files within a VPK!");           }
void CVPKFileSystem::Twstat(xhnd hnd, stat_t* stat, Rwstat_fn callback)                                   { callback("Cannot modify files within a VPK!");           }





#include <dy_ustack.h>

// VPK Archivist Frame 

// Directory ID
typedef uint32_t vpkarc_frame_did;

// VPK Archivist Frame Directory & Entry
struct vpkarc_frame_dir
{
	vpkarc_frame_did id;
	vpkarc_frame_did parent;

	xstr_t name;

	std::unordered_map<xstr_t, vpkarc_frame_dir*, xstrhash_t, xstrequality_t> children;
};

// VPK Archivist Frame File
struct vpkarc_frame_file
{
	vpkarc_frame_did parent;

	xstr_t name;

	vpk_direntry* entry;
};

class CVPKArchivistFrame
{
public:

	CVPKArchivistFrame();
	~CVPKArchivistFrame();

	// Ensure this directory exists and get its id
	vpkarc_frame_did EnsureDirectory(int nwalk, xstr_t walk);
	void AddFile(vpkarc_frame_did dir, xstr_t name, vpk_direntry* entry);

//private:
	dy_ustack<vpkarc_frame_dir> m_tree;
	dy_ustack<vpkarc_frame_file> m_files;

};

CVPKArchivistFrame::CVPKArchivistFrame()
{
	// Zero is always our root directory
	vpkarc_frame_dir* root = m_tree.push(vpkarc_frame_dir());
	// FIXME: hhhhelp
	new(root) vpkarc_frame_dir;
	root->name = xstrdup("/");
	root->id = 0;
	root->parent = 0;
}

CVPKArchivistFrame::~CVPKArchivistFrame()
{
	for (auto d : m_tree)
	{
		free(d->name);
	}
}


vpkarc_frame_did CVPKArchivistFrame::EnsureDirectory(int nwalk, xstr_t walk)
{
	// First direntry is the root
	vpkarc_frame_dir* cd = m_tree.first();


	// Roll over the walk
	int i = 0;
	for (xstr_t dir = walk; i < nwalk; dir = xstrnext(dir))
	{
		// Does this directory exist?
		auto f = cd->children.find(dir);
		if (f == cd->children.end())
		{
			// Nope! Let's make it!
			vpkfs_deid id = m_tree.count;
			vpkarc_frame_dir* nd = m_tree.push(vpkarc_frame_dir());
			// FIXME: hhhhelp
			new(nd) vpkarc_frame_dir();
			nd->id = id;
			nd->parent = cd->id;
			nd->name = xstrdup(dir);

			cd->children.insert({ nd->name, nd });

			cd = nd;

			i++;
			continue;
		}
		else
		{
			// Rad! It's here!
			cd = f->second;

		}

		i++;
	}


	return cd->id;
}


void CVPKArchivistFrame::AddFile(vpkarc_frame_did dir, xstr_t name, vpk_direntry* entry)
{
	m_files.push({ dir, name, entry });
}


void printDir(vpkarc_frame_dir* dir, int t)
{
	for (int i = 0; i < t; i++)
		fputc('\t', stdout);
	xstrprint(dir->name);
	fputc('\n', stdout);
	for (auto& p : dir->children)
	{
		printDir(p.second, t + 1);
	}
}

// VPK Archivist

CVPKArchivist::CVPKArchivist(const char* path)
{
	vpkerr err = ReadDir(path);
	OpenPacks(path);
}

CVPKArchivist::~CVPKArchivist()
{
	FreeTree();
	ClosePacks();
}


void CVPKArchivist::FreeTree()
{
	for (int i = 0; i < m_direntrycount; i++)
	{
		free(m_direntries[i].name);
	}

	m_direntrycount = 0;
	delete[] m_direntries;

	m_filecount = 0;
	delete[] m_files;

	m_directorycount = 0;
	delete[] m_directories;
}

vpkerr CVPKArchivist::ReadDir(const char* path)
{
	FILE* f = fopen(path, "rb");
	if (!f)
		return "The dir file does not exist!";

	// Get total size
	fseek(f, 0, SEEK_END);
	size_t fsz = ftell(f);
	fseek(f, 0, SEEK_SET);

	if (fsz < sizeof(vpk_v1_header))
	{
		fclose(f);
		return "The dir file is smaller than a vpk 1 header!";
	}
	if (fsz > UINT32_MAX)
	{
		fclose(f);
		return "The dir file is far too large!";
	}

	// Read the header into mem
	char headerbuf[sizeof(vpk_v2_header)];
	fread(&headerbuf[0], 1, sizeof(vpk_v2_header), f);
	vpk_v1_header* v1h = reinterpret_cast<vpk_v1_header*>(&headerbuf[0]);


	// Check the signature
	if (v1h->signature != 0x55aa1234)
	{
		fclose(f);
		return "The dir file's signature is invalid!";
	}


	size_t off = 0;

	if (v1h->version == 1)
	{
		// Version 1
		off += sizeof(vpk_v1_header);

		// We checked the length earlier
	}
	else if (v1h->version == 2)
	{
		// Version 2
		off += sizeof(vpk_v2_header);

		// Check the length
		if (fsz < sizeof(vpk_v2_header))
		{
			fclose(f);
			return "The dir file reports as vpk 2, but is too small!";
		}
	}
	else
	{
		// Version Invalid!
		fclose(f);
		return "The dir file reports an invalid or unsupported version!";
	}

	// Read the tree into mem
	char* buf = (char*)malloc(v1h->treedictsize);
	fseek(f, off, SEEK_SET);
	fread(buf, 1, v1h->treedictsize, f);

	char* cur = buf;
	char* end = cur + fsz;

	CVPKArchivistFrame frame;

	// Start pulling out our directory entries
	uint64_t preload_total = 0;
	for (;;)
	{
		char* ext = cur;
		for (; cur < end && *cur; cur++);
		size_t extlen = cur - ext;
		cur++;
		if (extlen == 0) break;

		// Check the extension's len
		if (extlen > VPK_MAX_EXTENSION_LEN)
		{
			free(buf); fclose(f);
			return "Dir entry's file extension is way too long!";
		}


		for (;;)
		{
			char* directory = cur;
			for (; cur < end && *cur; cur++);
			size_t directorylen = cur - directory;
			cur++;
			if (directorylen == 0) break;

			// Check the directory's len
			if (directorylen > VPK_MAX_DIRECTORY_LEN)
			{
				free(buf); fclose(f);
				return "Dir entry's directory path is way too long!";
			}

			// Chop up the directory into something like a 9P Walk
			// TODO: Is there something better we can do for this?
			int nwalk = 0;
			xstr_t walk = 0;
			xstrwalkfrompath(directory, nwalk, walk);

			// Get our walk's ID
			vpkarc_frame_did dirid = frame.EnsureDirectory(nwalk, walk);

			// FIXME: Change 'xstrwalkfrompath' so this can live on the stack or something!
			free(walk);


			for (;;)
			{
				char* filetitle = cur;
				for (; cur < end && *cur; cur++);
				size_t filetitlelen = cur - filetitle;
				cur++;
				if (filetitlelen == 0) break;
		
				// Check the title's len
				if (filetitlelen > VPK_MAX_DIRECTORY_LEN)
				{
					free(buf); fclose(f);
					return "Dir entry's file title is way too long!";
				}

				
				if (extlen == 1 && *ext == ' ')
				{
					puts("NO ext!");
				}


				// Get the direntry
				vpk_direntry* de = reinterpret_cast<vpk_direntry*>(cur);
				cur += sizeof(vpk_direntry) + de->preload_length;
#if 0
				Check if cur is still within the file!
#endif
				// All dir entries have a terminator at the end
				if (de->terminator != 0xFFFF)
				{
					free(buf); fclose(f);
					return "The dir entry is missing a terminator!";
				}


				// Make some space for our filename
				int filenamelen = filetitlelen + 1 + extlen;
				xstr_t filename = (xstr_t)malloc(filenamelen + sizeof(xstrlen_t));
				filename->len = filenamelen;
				char* fn = filename->str();

				// Copy in the title '.' and extension
				strncpy(fn, filetitle, filetitlelen);
				fn[filetitlelen] = '.';
				strncpy(&fn[filetitlelen+1], ext, extlen);

				// Track it
				frame.AddFile(dirid, filename, de);


				// AddFile will take ownership of filename. No need to free
			}

			// Done with this dir
		}

		// Done with this extension
	}

	if (preload_total > UINT32_MAX)
	{
		free(buf); fclose(f);
		return "Too much preload data!";
	}



	printDir(frame.m_tree.first(), 0);
	/*
	for (auto f : frame.m_files)
	{
		xstrprint(f->name);
		fputc('\n', stdout);
	}
	*/

	// Allocate up what we need for working with this fs

	m_direntrycount = frame.m_files.count + frame.m_tree.count;
	m_direntries = new vpkfs_direntry[m_direntrycount];

	m_directorycount = frame.m_tree.count;
	m_directories = new vpkfs_dir[m_directorycount];

	m_filecount = frame.m_files.count;
	m_files = new vpkfs_file[m_filecount];
	
	// Begin filling in data

	// Recreate the tree's direntries
	int i = 0;
	for (auto d : frame.m_tree)
	{
		vpkfs_direntry* de = &m_direntries[i];
		de->name = d->name;
		de->node = &m_directories[i];
		de->parent = &m_direntries[d->parent];

		vpkfs_dir* parent = reinterpret_cast<vpkfs_dir*>(de->parent->node);
		assert(parent->is_dir);

		if (parent != &m_directories[i])
			parent->children.insert({ de->name, de });
		i++;
	}

	// Recreate the files
	int dircount = i;
	i = 0;
	uint64_t preload_offset = 0;
	for (auto d : frame.m_files)
	{
		int j = dircount + i;
		
		vpkfs_file* file = &m_files[i];
		file->archive_index = d->entry->archive_index;
		file->data_length = d->entry->data_length;
		file->data_offset = d->entry->data_offset;
		file->preload_length = d->entry->preload_length;
		file->preload_offset = preload_offset;
		preload_offset += file->preload_length;

		vpkfs_direntry* de = &m_direntries[j];
		de->name = d->name;
		de->node = file;
		de->parent = &m_direntries[d->parent];

		// TODO: copy in data
		vpkfs_dir* parent = reinterpret_cast<vpkfs_dir*>(de->parent->node);
		assert(parent->is_dir);

		parent->children.insert({ de->name, de });
		i++;
	}

	
	// Create the preload data
	if (preload_total > 0)
	{
		m_preloadsize = preload_total;
		m_preloaddata = (char*)malloc(preload_total);
		char* p = m_preloaddata;
		for (auto d : frame.m_files)
		{
			vpk_direntry* entry = d->entry;
			
			uint16_t pl = entry->preload_length;
			if (pl == 0)
				continue;

			memcpy(p, entry + 1, pl);
			p += pl;
		}
	}
	else
	{
		m_preloadsize = 0;
		m_preloaddata = 0;
	}

	// Clear the frame as to not lose our names!
	// FIXME: We're leaking since m_tree cannot call deconstructors!
	for (auto d : frame.m_tree)
		d->children.clear();
	frame.m_tree.clear();
	frame.m_files.clear();

	free(buf);
	fclose(f);

	return 0;
}

void CVPKArchivist::ClosePacks()
{
	for (int i = 0; i < m_archivecount; i++)
	{
		fclose(m_archives[i]);
	}
	m_archivecount = 0;
	free(m_archives);
}

void CVPKArchivist::OpenPacks(const char* path)
{

	// Prep the string for having ID's inserted
	char str[512 + 1];
	memset(&str[0], 0, sizeof(str));
	strncpy(&str[0], path, 512);
	char* part = strstr(str, "dir");

	if (!part)
	{
		// No dir?
		m_archivecount = 0;
		m_archives = 0;
		return;
	}

	// FIXME: This is terrible and technically not format compliant
	//        VPK's can specify an archive with seemingly any ID
	//        A direntry could have an ID of 100 and only have one archive in total

	dy_ustack<FILE*> arstack;
	for (int i = 0; i < 1000; i++)
	{
		// Put the id into the path
		int d = i;
		for (int k = 3; k > 0; k--)
		{
			int e = d % 10;
			d /= 10;
			part[k-1] = e + '0';
		}
		puts(str);
		FILE* arch = fopen(str, "rb");
		if (arch == 0)
			break;
		arstack.push(arch);
	}
	printf("All packs open");

	m_archivecount = arstack.count;
	m_archives = arstack.packed();
}


void CVPKArchivist::ReadFile(vpkfs_file* file, void* dest, uint64_t offset, uint32_t count)
{

}
