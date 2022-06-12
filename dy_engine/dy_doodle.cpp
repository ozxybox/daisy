#include "dy_doodle.h"
#include "dy_ustack.h"
#include "dy_render.h"
#include "dy_engine.h"
#include "dy_shader.h"
#include "dy_primitives.h"
#include "dy_brush.h"
#include <math.h>

enum
{
	DOODLE_LINE,
	DOODLE_BOX,
	DOODLE_SPHERE,
	DOODLE_FACE,
};

struct doodle_command
{
	int type;
	float expiration;
	vec4 color;

	union
	{
		struct
		{
			mat4 model;
		};
		
		struct {
			dy_vtxbuf* vb;
			dy_idxbuf* ib;
			//dy_mesh mesh;
		};
	};

};

static dy_ustack<doodle_command*>* s_doodleCommands = 0;

static dy_mesh s_doodleLineMesh;
static dy_mesh s_doodleBoxMesh;
static dy_mesh s_doodleSphereMesh;

static dy_mesh s_doodleDynamicMesh;
#include <stdio.h>

void dy_doodle_init()
{
	// Dynamic
	s_doodleDynamicMesh.vbo = dy_render_create_vbo_dynamic();
	s_doodleDynamicMesh.ibo = dy_render_create_ibo_dynamic();
	s_doodleDynamicMesh.elements = 0;

	// Line
	s_doodleLineMesh.vbo = dy_render_create_vbo(&g_primitiveLine.vb);
	s_doodleLineMesh.ibo = dy_render_create_ibo(&g_primitiveLine.ib);
	s_doodleLineMesh.elements = g_primitiveLine.ib.used;

	// Box
	s_doodleBoxMesh.vbo = dy_render_create_vbo(&g_primitiveBox.vb);
	s_doodleBoxMesh.ibo = dy_render_create_ibo(&g_primitiveBox.ib);
	s_doodleBoxMesh.elements = g_primitiveBox.ib.used;

	// Sphere
	s_doodleSphereMesh.vbo = dy_render_create_vbo(&g_primitiveSphere.vb);
	s_doodleSphereMesh.ibo = dy_render_create_ibo(&g_primitiveSphere.ib);
	s_doodleSphereMesh.elements = g_primitiveSphere.ib.used;


	s_doodleCommands = new dy_ustack<doodle_command*>();
}

void dy_doodle_cmd_draw(doodle_command* cmd)
{
	dy_shader_set(DY_SHADERPARAM_COLOR, &cmd->color);
	mat4 model;

	switch (cmd->type)
	{
	case DOODLE_LINE:
		dy_shader_set(DY_SHADERPARAM_MODEL, &cmd->model);
		dy_render_draw_mesh(&s_doodleLineMesh);
		break;

	case DOODLE_BOX:
		dy_shader_set(DY_SHADERPARAM_MODEL, &cmd->model);
		dy_render_draw_mesh(&s_doodleBoxMesh);
		break;

	case DOODLE_SPHERE:
		dy_shader_set(DY_SHADERPARAM_MODEL, &cmd->model);
		dy_render_draw_mesh(&s_doodleSphereMesh);
		break;

	case DOODLE_FACE:
	{
		// Load the mesh
		dy_render_fill_vbo_dynamic(s_doodleDynamicMesh.vbo, cmd->vb);
		dy_render_fill_ibo_dynamic(s_doodleDynamicMesh.ibo, cmd->ib);
		s_doodleDynamicMesh.elements = cmd->ib->used;

		// Draw it
		model = mat4::identity();
		dy_shader_set(DY_SHADERPARAM_MODEL, &model);
		dy_render_draw_mesh(&s_doodleDynamicMesh);
	}

		break;

	default:
		break;
	}
}

void dy_doodle_cmd_delete(doodle_command* cmd)
{
	switch (cmd->type)
	{
	case DOODLE_LINE:
		break;

	case DOODLE_BOX:
		break;

	case DOODLE_SPHERE:
		break;

	case DOODLE_FACE:
		delete cmd->vb;
		delete cmd->ib;
		break;

	default:
		break;
	}
}

void dy_doodle_render()
{
	assert(s_doodleCommands);

	float time = dy_curtime();

	// Nothing new? Nothing to do.
	if (s_doodleCommands->count == 0)
		return;

	dy_ustack<doodle_command*>* remaining = new dy_ustack<doodle_command*>();
	dy_ustack<doodle_command*>* cmdlist = s_doodleCommands;
	s_doodleCommands = remaining;

	for (doodle_command** pcmd : *cmdlist)
	{
		doodle_command* cmd = *pcmd;


		bool survives = false;
		if (cmd->expiration < 0)
		{
			// Draw for this frame only

		}
		else if(cmd->expiration > time)
		{
			// Still alive
			survives = true;
		}
		
		// Draw it
		dy_doodle_cmd_draw(cmd);

		if(survives)
			remaining->push(cmd);
		else
		{
			dy_doodle_cmd_delete(cmd);
			delete cmd;
		}
	}

	// Delete the old list
	delete cmdlist;
	
}



// Lives for "decay" seconds
void dy_doodle_line(vec3 start, vec3 end, vec4 color, float width, float decay)
{
	assert(s_doodleCommands);
	assert(width != 0);
	if (width == 0)
		return;


	vec3 delta = end - start;

	// Find a line orthogonal from the delta
	vec3 orth1 = delta.cross({ 1, 0, 0 });

	// Cross product can return a 0 vector if the parameters match, so we have to find an alternative
	if (orth1.x == 0 && orth1.y == 0 && orth1.z == 0)
		orth1 = delta.cross({ 0, 1, 0 });

	// Find a second line orthogonal from the delta and our first line
	vec3 orth2 = delta.cross(orth1);

	// Scale both ortho lines to the width
	orth1 *= 0.5 * width / orth1.mag();
	orth2 *= 0.5 * width / orth2.mag();

	mat4 model =
	{{orth1, 0.0},
	 {orth2, 0.0},
	 {delta, 0.0},
	 {start, 1.0}};

	doodle_command* cmd = new doodle_command{};
	cmd->type = DOODLE_LINE;
	cmd->color = color;
	cmd->model = model;

	if (decay < 0)
		cmd->expiration = -1;
	else
		cmd->expiration = dy_curtime() + decay;

	// Commit the command
	s_doodleCommands->push(cmd);
}



void dy_doodle_aabb(dy_aabb aabb, vec4 color, float bloat, float decay)
{
	assert(s_doodleCommands);

	vec3 origin = aabb.mins;
	vec3 size = aabb.maxs - aabb.mins;

	// Bloat it up
	origin -=     {bloat, bloat, bloat};
	size   += vec3{bloat, bloat, bloat} * 2.0f;


	mat4 model =
	{{  size.x,      0.0,      0.0, 0.0},
	 {     0.0,   size.y,      0.0, 0.0},
	 {     0.0,      0.0,   size.z, 0.0},
	 {origin.x, origin.y, origin.z, 1.0}};


	doodle_command* cmd = new doodle_command{};
	cmd->type = DOODLE_BOX;
	cmd->color = color;
	cmd->model = model;

	if (decay < 0)
		cmd->expiration = -1;
	else
		cmd->expiration = dy_curtime() + decay;

	// Commit the command
	s_doodleCommands->push(cmd);
}


void dy_doodle_sphere(vec3 origin, float radius, vec4 color, float decay)
{
	assert(s_doodleCommands);

	mat4 model =
	{{  radius,      0.0,      0.0, 0.0},
	 {     0.0,   radius,      0.0, 0.0},
	 {     0.0,      0.0,   radius, 0.0},
	 {origin.x, origin.y, origin.z, 1.0}};


	doodle_command* cmd = new doodle_command{};
	cmd->type = DOODLE_SPHERE;
	cmd->color = color;
	cmd->model = model;

	if (decay < 0)
		cmd->expiration = -1;
	else
		cmd->expiration = dy_curtime() + decay;

	// Commit the command
	s_doodleCommands->push(cmd);
}


// TODO: if the model sticks around for more than a frame, load it into a proper vbo instead of reuploading it every frame
void dy_doodle_rface(dy_rface* face, vec4 color, float dbloat, float decay)
{
	assert(s_doodleCommands);

	int vc = 0;
	dy_rhalfedge* first = face->first, * he = first;
	do
	{
		vc++;
		he = he->next;
	} while (he != first);

	dy_vtxbuf* vb = new dy_vtxbuf(vc);
	dy_idxbuf* ib = new dy_idxbuf((vc - 2) * 3);

	first = face->first;
	he = first;
	do
	{
		dy_vertex v;
		v.color = {1,1,1};
		v.pos = *he->vert->pos + face->plane->norm * dbloat;
		v.uv = {0,0};
		v.norm = face->plane->norm;
		vb->push(&v);
		he = he->next;
	} while (he != first);
	
	ib->push_convexpoly(0, vc);

	doodle_command* cmd = new doodle_command{};
	cmd->type = DOODLE_FACE;
	cmd->color = color;
	cmd->vb = vb;
	cmd->ib = ib;


	if (decay < 0)
		cmd->expiration = -1;
	else
		cmd->expiration = dy_curtime() + decay;

	// Commit the command
	s_doodleCommands->push(cmd);
}
