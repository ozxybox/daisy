#pragma once
#include "dy_render.h"
#include "dy_brush.h"
#include "dy_ustack.h"
#include "world.h"

union dy_frustrum
{
	dy_bplane planes[6];
	struct {
		dy_bplane near;
		dy_bplane far;
		dy_bplane top;
		dy_bplane bottom;
		dy_bplane left;
		dy_bplane right;
	};
};

void mesh_drawdata(dy_rmesh* mesh, dy_vtxbuf*& ovb, dy_idxbuf*& oib);

void fill_world(dy_ustack<dy_bsolid>& solids);

void draw_world(dy_frustrum& frustrum);

void clear_world();
