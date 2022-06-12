#include "worldrender.h"
#include <dy_texture.h>
#include "dy_shader.h"
#include "dy_ustack.h"
#include "dy_doodle.h"
#include <math.h>
#include "world.h"



extern vec2 mesh_caluv(vec3 norm, vec3 absnorm, vec3 pos, vec4 txdata);

void mesh_drawdata(dy_rmesh* mesh, dy_vtxbuf*& ovb, dy_idxbuf*& oib)
{
	// TODO: make a vtxbuf & idxbuf that uses ustack for the internal representation
	unsigned int vcount = 0;
	unsigned int icount = 0;
	for (int i = 0; i < mesh->face_count; i++)
	{
		dy_rface* face = mesh->faces[i];
		int vc = 0;
		dy_rhalfedge* first = face->first, * he = first;
		do
		{
			vc++;
			he = he->next;
		} while (he != first);

		vcount += vc;
		icount += (vc - 2) * 3;
	}

	dy_vtxbuf* vb = new dy_vtxbuf(vcount);
	dy_idxbuf* ib = new dy_idxbuf(icount);

	int plidx = 0;
	for (int i = 0; i < mesh->face_count; i++)
	{
		dy_rface* face = mesh->faces[i];

		// FIXME: AAAAA
		//vec3 color = dy_color_hsv(fmod(face->first->vert->pos->mag(), 1.0), 0.5, 1.0);

		vec4 txdata = { 0, 0, 1.0/128.0, 1.0/128.0 };
		vec3 absnorm = face->plane->norm.abs();

		int start = vb->used;
		int hei = 0;
		dy_rhalfedge* first = face->first, * he = first;
		do
		{
			dy_vertex v;

			v.color = {1.0,1.0,1.0};
			//v.color = color;
			//v.color = (0.5 * face->plane->norm + vec3{ 0.5f,0.5f,0.5f } ) * 0.75 + vec3{0.25, 0.25, 0.25};// {1.0, 1.0, 1.0};
			v.pos = *he->vert->pos;
			v.uv = mesh_caluv(face->plane->norm, absnorm, v.pos, txdata);
			v.norm = face->plane->norm;
			vb->push(&v);
			hei++;
			he = he->next;
		} while (he != first);
		ib->push_convexpoly(start, hei);

		plidx++;
	}

	ovb = vb;
	oib = ib;
}



void clear_world()
{
#if 0
	for (brush_draw* bd : s_brush_draws)
	{
		dy_render_destroy_ibo(bd->ibo);
		dy_render_destroy_vbo(bd->vbo);
	}
	s_brush_draws.clear();
#else
	assert(0);
#endif
}

extern dy_netdb_tree s_tree;
void brush_remesh(dy_netdb_objref<dy_brush> brushref)
{
	dy_brush* brush = brushref;

	dy_array<dy_netdb_obj*> childIDs = s_tree.children(brushref.id());


	dy_array<dy_netdb_objref<dy_bplane>>* planeRefs = (dy_array<dy_netdb_objref<dy_bplane>>*)&childIDs;
	dy_rmesh mesh = dy_bsolid_mesh(*planeRefs);

	// Convert the mesh into something drawable
	dy_vtxbuf* vb;
	dy_idxbuf* ib;
	mesh_drawdata(&mesh, vb, ib);


	dy_vbo vbo = dy_render_create_vbo(vb);
	dy_ibo ibo = dy_render_create_ibo(ib);

	dy_mesh* bd = new dy_mesh{ vbo, ibo, ib->used };

	delete vb;
	delete ib;

	brush->mesh = mesh;
	brush->draw = bd;
}

#if 0

void fill_world(dy_ustack<dy_bsolid>& solids)
{

	for (dy_bsolid* solid : solids)
	{

		
	//	dy_rmesh_clear(&mesh);


		dy_brush b;
		b.solid = *solid;
		brush_remesh(&b);
		s_brushes.push(b);


	}

}
#endif


void draw_world(dy_frustrum& frustrum)
{
	dy_ustack<dy_netdb_objref<dy_brush>>& brushes = *(dy_ustack<dy_netdb_objref<dy_brush>>*)&s_tree.objectsForType(DY_NETDB_TYPE_SOLID);

	dy_ustack<dy_mesh*> candidates;

	for (dy_netdb_objref<dy_brush>* bref : brushes) {

		//https://cgvr.informatik.uni-bremen.de/teaching/cg_literatur/lighthouse3d_view_frustum_culling/index.html
		
		dy_brush* b = bref->ref();

		const dy_aabb& aabb = b->mesh.aabb;
		bool inside = true;
		for (int j = 0; j < 6; j++)
		{
			dy_bplane* pl = frustrum.planes + j;
			
			vec3 norm = pl->norm;

			vec3 n = aabb.maxs;
			if (norm.x >= 0)
				n.x = aabb.mins.x;
			if (norm.y >= 0)
				n.y = aabb.mins.y;
			if (norm.z >= 0)
				n.z = aabb.mins.z;

			if (pl->norm.dot(n) > pl->d)
			{
				// Outside!
				inside = false;
				break;
			}
		}

		if (inside)
			candidates.push(b->draw);
//		else
//			dy_doodle_aabb(aabb, vec4{1,0,0,0.5}, 0, -1);



	}
	
	//printf("%0.2f\n", candidates.count / (float)s_brush_draws.count * 100);

	// Draw mesh
	for (dy_mesh** pbd : candidates)
	{
		dy_mesh* bd = *pbd;
		dy_render_draw_mesh(bd->vbo, bd->ibo, 0, bd->elements);
	}
}



