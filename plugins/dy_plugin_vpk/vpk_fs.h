#pragma once
#include "X9PFileSystem.h"
#include <unordered_map>


// This ID is an index of a directory entry within the fs tree
typedef uint32_t vpkfs_deid;



struct vpkfs_node
{
	bool is_dir;
};

struct vpkfs_direntry
{
	xstr_t name;
	vpkfs_direntry* parent;
	vpkfs_node* node;
};


// Such a long type name!
typedef std::unordered_map<xstr_t, vpkfs_direntry*, xstrhash_t, xstrequality_t> vpkfs_map;

struct vpkfs_dir : public vpkfs_node
{
	vpkfs_dir() { is_dir = true; }
	vpkfs_map children;
};

struct vpkfs_file : public vpkfs_node
{
	vpkfs_file() { is_dir = false; }

	uint16_t archive_index; // If this is equal to VPK_ARCHIVE_DATA_FOLLOWS, our one element archive is right after this direntry
	
	uint16_t preload_length;
	uint32_t preload_offset;

	uint32_t data_length;   // Length of the data
	uint32_t data_offset;   // Offset from start of archive
};



// VPK Archive Manager
typedef const char* vpkerr;
class CVPKArchivist
{
public:
	CVPKArchivist(const char* path);
	~CVPKArchivist();

protected:

	void FreeTree();
	vpkerr ReadDir(const char* path);


	void ClosePacks();
	// Open up all packs associated with this path
	void OpenPacks(const char* path);

	void ReadFile(vpkfs_file* file, void* dest, uint64_t offset, uint32_t count);


	FILE** m_archives;
	int m_archivecount;

	char* m_preloaddata;
	uint32_t m_preloadsize;

	vpkfs_direntry* m_direntries;
	uint32_t m_direntrycount;

	vpkfs_dir* m_directories;
	uint32_t m_directorycount;

	vpkfs_file* m_files;
	uint32_t m_filecount;

};


// VPK 9P File System
class CVPKFileSystem : public X9PFileSystem, protected CVPKArchivist
{
public:

	CVPKFileSystem(const char* vpk_path);


	// Since xhids are uint64's, we can store the pointer right in the id
	vpkfs_deid GetEntryID(vpkfs_direntry* node);
	vpkfs_direntry* GetIDEntry(vpkfs_deid id);
	void TagHND(xhnd hnd, vpkfs_direntry* de);
	vpkfs_direntry* HNDValue(xhnd hnd);


	vpkfs_direntry* GetNodeChild(vpkfs_node* node, xstr_t name);

	// RPC calls
	virtual void Tattach(xhnd hnd, xhnd ahnd, xstr_t username, xstr_t accesstree, Rattach_fn callback);
	virtual void Twalk(xhnd hnd, xhnd newhnd, uint16_t nwname, xstr_t wname, Rwalk_fn callback);
	virtual void Topen(xhnd hnd, openmode_t mode, Ropen_fn callback);
	virtual void Tread(xhnd hnd, uint64_t offset, uint32_t count, Rread_fn callback);
	virtual void Tstat(xhnd hnd, Rstat_fn callback);

	// Stubs
	virtual void Tclunk(xhnd hnd, Rclunk_fn callback);
	virtual void Tcreate(xhnd hnd, xstr_t name, dirmode_t perm, openmode_t mode, Rcreate_fn callback);
	virtual void Twrite(xhnd hnd, uint64_t offset, uint32_t count, void* data, Rwrite_fn callback);
	virtual void Tremove(xhnd hnd, Rremove_fn callback);
	virtual void Twstat(xhnd hnd, stat_t* stat, Rwstat_fn callback);


};