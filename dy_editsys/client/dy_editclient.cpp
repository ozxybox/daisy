#include "dy_editsys.h"
#include "world.h"
#include "dy_ustack.h"
#include "X9PClient.h"
#include "X9PFileSystem.h"
#include "XIO.h"
#include "socket/TCPSocket.h"
#include <chrono>
#include <thread>
#include "dy_netdb.h"

extern dy_netdb_tree s_tree;

struct finfo
{
	xstr_t str;
	qid_t qid;
};

void get_listing(XFile& dir, dy_ustack<finfo>& paths)
{
	
	uint32_t iou = 2048;

	dir.Open(X9P_OPEN_READ, [&](xerr_t err, qid_t* qid, uint32_t iounit) {
		if (err) puts(err);
		assert(!err);

		iou = iounit;
	});

	dir.Await();

	uint64_t offset = 0;
	bool morefiles = true;
	while (morefiles)
	{

		dir.Read(offset, iou, [&](xerr_t err, uint32_t count, void* data) {
			if (count == 0)
			{
				morefiles = false;
				return;
			}
			
			offset += count;
			char* dc = (char*)data;

			uint64_t read = 0;

			while (read < count)
			{

				stat_t* s = (stat_t*)(dc + read);
				read += s->size;

				paths.push({ xstrdup(s->name()), s->qid });

			}
		});

		dir.Await();
	}

}


void fill_world()
{
	dy_ustack<dy_netdb_obj*>& solids = s_tree.objectsForType(DY_NETDB_TYPE_SOLID);
	for (dy_netdb_obj** solid : solids)
	{
		brush_remesh({ *solid });
	}

}

void download_world(XFile worldhnd)
{
	
	dy_ustack<finfo> solidpaths;
	get_listing(worldhnd, solidpaths);


	
	for(finfo* f : solidpaths)
	{

		// Should we use the Walk's qid path instead? Probably
		int solidid = f->qid.path;
		s_tree.forceCreate(solidid, 0, DY_NETDB_TYPE_SOLID, new dy_brush);


		// Open the solid's directory
		XFile solid;
		worldhnd.Walk(solid, f->str, [&](xerr_t err, uint16_t nwqid, qid_t* wqid) {
			if (err) puts(err);
		});

		/***************/
		worldhnd.Await();

		// Get the solid's plane listing 
		dy_ustack<finfo> planepaths;
		get_listing(solid, planepaths);


		for (finfo* g : planepaths)
		{
			int planeid = g->qid.path;

			// Walk to file
			XFile plane;
			solid.Walk(plane, g->str, [&](xerr_t err, uint16_t nwqid, qid_t* wqid) {
				if (err) puts(err);
				});

			// Open the file
			plane.Open(X9P_OPEN_READ, [&](xerr_t err, qid_t* qid, uint32_t iounit) {
				if (err) puts(err);
				});

			// Read data from plane
			plane.Read(0, sizeof(dy_bplane), [&](xerr_t err, uint32_t count, void* buf) {
				if (err)
				{
					puts(err);
					return;
				}
				if (count != sizeof(dy_bplane)) {
					puts("bad size");
					return;
				}
				//assert(count == sizeof(dy_bplane));

				dy_bplane* bp = new dy_bplane;
				*bp = *reinterpret_cast<dy_bplane*>(buf);


				s_tree.forceCreate(planeid, solidid, DY_NETDB_TYPE_PLANE, bp);

			});

			//plane.Clunk();


			plane.Await();
		}
		/***************/
		solid.Await();

		//solid.Clunk();

		// Cleanup
//		for (finfo* g : planepaths)
//			free(g->str);
//		free(f->str);

		/***************/
		solid.Await();
	}

	s_tree.rebuild();
	
	printf("Downloaded %u solids and %u planes!\n", s_tree.objectsForType(DY_NETDB_TYPE_SOLID).count, s_tree.objectsForType(DY_NETDB_TYPE_PLANE).count);

	fill_world();



}
#include "dy_ot.h"
XFile s_root;
X9PClient* cl ;
XAuth auth = { cl, XSTRL("user"), 0};

void dy_editsys_begin_client(const char* ip, const char* port, const char* username)
{
	SystemInitSocket();
	xerr_t err;
	cl = new X9PClient;
	auth.client = cl;
	auth.uname = xstrdup(username);
	auth.id = 0;

retryConnect:
	err = cl->Begin(ip, port);
	if (err)
	{
		puts(err);
		printf("\tRETRYING CONNECTION IN 4 SECONDS");
		
		for (int i = 0; i < 4; i++)
		{
			std::this_thread::sleep_for(std::chrono::seconds(1));
			putchar('.');
		}
		putchar('\n');

		goto retryConnect;
	}



	
	s_root.Attach(&auth, XSTRL("/"), cl, [&](xerr_t err, qid_t* q) {
		if (err) { puts(err); return; };
		printf("Connected!\n");
	});

	// World
	XFile worldhnd;
	s_root.Walk(worldhnd, XSTRL("solids"), [&](xerr_t err, uint16_t nwqid, qid_t* wqid) {
		if (err) { puts(err); return; };
	});

	s_root.Await();

	// Download the world
	download_world(worldhnd);


#if 0
	XFile timelinehnd;
	s_root.Walk(timelinehnd, XSTRL("timeline"), [&](xerr_t err, uint16_t nwqid, qid_t* wqid) {
		if (err) { puts(err); return; };
		});

	s_root.Await();

	timelinehnd.Create(XSTRL("update_plane"), X9P_DM_PERM_MASK, X9P_OPEN_EXEC, [&](xerr_t err, qid_t* qid, uint32_t iounit) {
		if (err) { puts(err); return; };

		printf("stamp id: %llu\n", qid->path);
		});
	
	dy_ot_data_update_plane up;
	up.target = 3;
	up.content = { {0,1,0}, 128 };
	timelinehnd.Write(0, sizeof(up), &up);

	timelinehnd.Clunk();

	timelinehnd.Await();
#endif


	/*
	for (int i = 0; i < 10; i++)
	{
		// Timeline
		XFile timelinehnd;
		root.Walk(timelinehnd, XSTRL("timeline"), [&](xerr_t err, uint16_t nwqid, qid_t* wqid) {
			if (err) { puts(err); return; };
		});

		root.Await();

		timelinehnd.Create(XSTRL("create_solid"), X9P_DM_PERM_MASK, X9P_OPEN_EXEC, [&](xerr_t err, qid_t* qid, uint32_t iounit) {
			if (err) { puts(err); return; };

			printf("stamp id: %llu\n", qid->path);
		});
		timelinehnd.Clunk();

		timelinehnd.Await();
	}
	*/


	
}

void dy_editsys_client_update()
{
	cl->ProcessPackets();
}
