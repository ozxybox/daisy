#pragma once
#include "dy_brush.h"
#include "dy_netdb.h"


struct dy_mesh;
struct dy_brush //: public dy_netdb_obj_base
{
	dy_rmesh mesh;
	dy_mesh* draw;

	dy_array<dy_netdb_objref<dy_bplane>> planes;
};

void brush_remesh(dy_netdb_objref<dy_brush> brushref);

struct dy_ray
{
	dy_ray(vec3 _start, vec3 _end);

	// results
	vec3 intersect;
	dy_rface* face;
	dy_netdb_objref<dy_brush> brush;

	// internal
	vec3 start;
	vec3 dir;
	float t;

};
void raytest(dy_ray* ray);



bool raytest_plane(dy_bplane* plane, dy_ray* ray);
