#pragma once
#include "dy_math.h"
#include "dy_halfedge.h"

struct dy_bplane
{
	vec3 norm;
	float d;

};

struct dy_bsolid
{
	dy_bplane* planes;
	int plane_count;
};

bool dy_bplane_intersect(dy_bplane* plane1, dy_bplane* plane2, dy_bplane* plane3, vec3* out);

dy_rmesh dy_bsolid_mesh(dy_bsolid* solid);
