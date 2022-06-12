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
#include <socket/TCPSocket.h>
#include "dy_editsys.h"

extern dy_netdb_tree s_tree;

#define SKIP_MAT 1
void add_ent_solids(KeyValue& world)
{
	
	for (int i = 0; i < world.childCount; i++)
	{
		KeyValue& solid = world.Get(i);
		if (strcmp("solid", solid.key.string) != 0)
			continue;
		//printf("Solid!\n");

#if SKIP_MAT
		bool skipit = false;
#endif
		dy_ustack<dy_bplane> sides;
		for (int k = 0; k < solid.childCount; k++)
		{
			KeyValue& side = solid.Get(k);
			if (side.key.string == 0)
			{
				puts("huh?");
				continue;
			}
			if (strcmp("side", side.key.string) != 0)
				continue;
			//printf("Side!\n");

#if SKIP_MAT
			KeyValue& material = side.Get("material");
			if (strcmp(material.value.string, "TOOLS/TOOLSSKIP") == 0 || strcmp(material.value.string, "TOOLS/TOOLSHINT") == 0)
			{
				skipit = true;
				break;
			}
#endif


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

#if SKIP_MAT
		if (skipit)
			continue;
#endif

		if (sides.count <= 4)
			continue;

		dy_netdb_obj* sobj = s_tree.create(0, DY_NETDB_TYPE_SOLID, 0);

		for (dy_bplane* plane : sides)
		{
			s_tree.create(sobj->id(), DY_NETDB_TYPE_PLANE, new dy_bplane(*plane));
		}
	}


}

int main(int argc, const char** args)
{
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


	// Add in the world's geo
	add_ent_solids(kv["world"]);


	// Add in all func_details
	for (int i = 0; i < kv.childCount; i++)
	{
		KeyValue& ent = kv.Get(i);
		if (strcmp("entity", ent.key.string) != 0)
			continue;

		KeyValue& classname = ent["classname"];
		if (classname.hasChildren || strcmp("func_detail", classname.value.string) != 0)
			continue;

		add_ent_solids(ent);
	}


	s_tree.rebuild();

	printf("Hosting %u solids and %u planes!\n", s_tree.objectsForType(DY_NETDB_TYPE_SOLID).count, s_tree.objectsForType(DY_NETDB_TYPE_PLANE).count);

	SystemInitSocket();
	dy_editsys_begin_server("27015");

	return 0;
}

