#pragma once
#include "dy_math.h"

struct dy_rface;
struct dy_rhalfedge;

struct dy_rvertex
{
	vec3*         pos;  // Position of this vertex
	dy_rhalfedge* edge; // One edge that stems out of this vertex
};

struct dy_rhalfedge
{
	dy_rhalfedge* next; // The next edge in the face
	dy_rhalfedge* pair; // Other half of this edge
	dy_rvertex*   vert; // The vertex this edge stems from
	dy_rface*     face; // The owning face
};

struct dy_bplane;
struct dy_rface
{
	dy_rhalfedge* first; // First half edge on the face
	dy_bplane* plane; // The brush plane this face belongs to
};

struct dy_rmesh
{
	dy_rface** faces;
	int face_count;
	vec3* points;
};


void dy_rmesh_clear(dy_rmesh* mesh);

// Cuts the face across the cutting plane
// Anything on the backside of the plane will be discarded
void dy_rface_cut(dy_rface* face, dy_bplane* cuttingplane);


