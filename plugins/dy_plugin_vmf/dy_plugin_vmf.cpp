#include "csg/dy_csg.h"
#include "csg/dy_brush.h"
#include "dy_ustack.h"
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <KeyValue.h>
#include <assert.h>
#include <XVirtualFileSystem.h>
#include <socket/TCPSocket.h>
#include <X9PServer.h>


typedef XVFSTypePointerFile<dy_bplane> VMFPlaneFile;


class VMFSolidPlanesDirectory : public XBaseVirtualFile
{
public:

	VMFSolidPlanesDirectory(dy_bsolid* solid)
		: m_solid(solid)
	{

		m_files = new direntry_t[m_solid->plane_count];
		for (int i = 0; i < m_solid->plane_count; i++)
		{
			direntry_t* de = &m_files[i];
			de->name = xstrfromd(i, 10);
			de->parent = this;
			de->node = new VMFPlaneFile(&m_solid->planes[i]);
		}
	}

	virtual ftype_t  Type()       { return X9P_FT_DIRECTORY; }

	// Directory Functions
	virtual xerr_t AddChild(xstr_t name, XVirtualFile* file, direntry_t** out) { if (out) *out = 0; return "Not allowed to add more planes!"; };
	virtual xerr_t RemoveChild(xstr_t name) { return "Not allowed to remove any planes!"; };
	virtual uint32_t ChildCount() { return m_solid->plane_count; }
	virtual xerr_t FindChild(xstr_t name, direntry_t** out)
	{
		int d = 0;
		if (!xstrtod(name, &d) || d < 0) return "File does not exist!";
		if (d >= m_solid->plane_count) return "Index out of range!";

		if (out)
			*out = &m_files[d];

		return 0;
	}
	virtual xerr_t GetChild(uint32_t index, direntry_t** out)
	{
		if (index >= ChildCount())
		{
			if (out) *out = 0;
			return "Index out of range!";
		}
		
		if(out) 
			*out = &m_files[index];

		return 0;
	}

private:
	direntry_t* m_files;
	dy_bsolid* m_solid;
};


class VMFSolidsDirectory : public XBaseVirtualFile
{
public:

	VMFSolidsDirectory(dy_bsolid* solids, uint32_t solidcount)
		: m_solids(solids), m_solidcount(solidcount)
	{

		m_files = new direntry_t[m_solidcount];
		for (int i = 0; i < m_solidcount; i++)
		{
			direntry_t* de = &m_files[i];
			de->name = xstrfromd(i, 10);
			de->parent = this;
			de->node = new VMFSolidPlanesDirectory(&m_solids[i]);
		}
	}

	virtual ftype_t  Type()       { return X9P_FT_DIRECTORY; }

	// Directory Functions
	virtual xerr_t AddChild(xstr_t name, XVirtualFile* file, direntry_t** out) { if (out) *out = 0; return "Not allowed to add more planes!"; };
	virtual xerr_t RemoveChild(xstr_t name) { return "Not allowed to remove any planes!"; };
	virtual uint32_t ChildCount() { return m_solidcount; }
	virtual xerr_t FindChild(xstr_t name, direntry_t** out)
	{
		int d = 0;
		if (!xstrtod(name, &d) || d < 0) return "File does not exist!";
		if (d >= m_solidcount) return "Index out of range!";

		if (out)
			*out = &m_files[d];

		return 0;
	}
	virtual xerr_t GetChild(uint32_t index, direntry_t** out)
	{
		if (index >= ChildCount())
		{
			if (out) *out = 0;
			return "Index out of range!";
		}
		
		if(out) 
			*out = &m_files[index];

		return 0;
	}

private:
	direntry_t* m_files;
	dy_bsolid* m_solids;
	uint32_t m_solidcount;
};

void add_ent_solids(dy_ustack<dy_bsolid>& solids, KeyValue& world)
{
	
	for (int i = 0; i < world.childCount; i++)
	{
		KeyValue& solid = world.Get(i);
		if (strcmp("solid", solid.key.string) != 0)
			continue;
		//printf("Solid!\n");

		dy_ustack<dy_bplane> sides;
		for (int k = 0; k < solid.childCount; k++)
		{
			KeyValue& side = solid.Get(k);
			if (strcmp("side", side.key.string) != 0)
				continue;
			//printf("Side!\n");

			KeyValue& plane = side.Get("plane");

			// FIX ME: ROTATE THE CAMERA, NOT THE WORLD
			vec3 points[3];
			//int it = sscanf(plane.value.string, "(%f %f %f) (%f %f %f) (%f %f %f)", &points[0].x, &points[0].y, &points[0].z, &points[1].x, &points[1].y, &points[1].z, &points[2].x, &points[2].y, &points[2].z);
			int it = sscanf(plane.value.string, "(%f %f %f) (%f %f %f) (%f %f %f)", &points[0].x, &points[0].z, &points[0].y, &points[1].x, &points[1].z, &points[1].y, &points[2].x, &points[2].z, &points[2].y);
			assert(it == 9);

			vec3 n = vec3::cross(points[0] - points[1], points[1] - points[2]).normalized();
			float d = vec3::dot(points[0], n);

			sides.push({n, d});
		}

		dy_bsolid s;
		s.planes = sides.packed();
		s.plane_count = sides.count;
		solids.push(&s);
	}
}

int main(int argc, const char** args)
{
	SystemInitSocket();

	XVirtualFileSystem vfs;

	XVirtualFile* root = vfs.RootDirectory()->node;

	const char* filename = args[1];// "sdk_dm_lockdown.vmf";



	FILE* f = fopen(filename, "rb");
	fseek(f, 0, SEEK_END);
	size_t sz = ftell(f);
	fseek(f, 0, SEEK_SET);

	char* m = (char*)calloc(sz + 1, 1);
	fread(m, 1, sz, f);
	fclose(f);


	KeyValueRoot kv(m);
	kv.Solidify();

	dy_ustack<dy_bsolid> solids;

	// Add in the world's geo
	add_ent_solids(solids, kv["world"]);


	// Add in all func_details
	for (int i = 0; i < kv.childCount; i++)
	{
		KeyValue& ent = kv.Get(i);
		if (strcmp("entity", ent.key.string) != 0)
			continue;

		KeyValue& classname = ent["classname"];
		if (classname.hasChildren || strcmp("func_detail", classname.value.string) != 0)
			continue;

		add_ent_solids(solids, ent);
	}

	for (dy_bsolid* solid : solids)
	{
		dy_rmesh mesh = dy_bsolid_mesh(solid);

	}

	dy_bsolid* solidspacked = solids.packed();


	XVFSDirectory* folder = new XVFSDirectory;
	VMFSolidsDirectory* solidDir = new VMFSolidsDirectory(solidspacked, solids.count);
	root->AddChild(XSTR_L("world1.vmf"), solidDir, 0);

	X9PServer sv;
	sv.Begin(nullptr, "27015", &vfs);

	printf("Begin!\n");

	while (1)
	{
		sv.AcceptConnections();
		sv.ProcessPackets();
	}
	return 0;
}

