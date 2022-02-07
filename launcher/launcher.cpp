#include "engine/dy_engine.h"
#include "engine/dy_render.h"
#include "engine/dy_texture.h"
#include "engine/dy_shader.h"
#include "csg/dy_csg.h"
#include "csg/dy_brush.h"
#include <stdio.h>
#include <time.h>
#include <math.h>

static const char* s_vertexshader =
"#version 330 core\n"
"layout(location = 0) in vec3 a_pos;"
"layout(location = 1) in vec3 a_color;"
"layout(location = 2) in vec2 a_uv;"
"uniform mat4 u_model;"
"uniform mat4 u_view;"
"uniform mat4 u_projection;"
"out vec3 v_color;"
"out vec2 v_uv;"
"void main(){"
"    v_color=a_color;"
"    v_uv=a_uv;"
"    gl_Position = u_projection * u_view * u_model * vec4(a_pos.xyz, 1.0);"
"}";
static const char* s_fragmentshader =
"#version 330 core\n"
"precision lowp float;"
"in vec3 v_color;"
"in vec2 v_uv;"
"out vec4 o_fragColor;"
"uniform sampler2D u_tex;"
"void main() {"
"    o_fragColor = vec4(v_color.xyz, 1.0) * vec4(texture(u_tex,v_uv).xyz, 1.0);"
"}";


static dy_shader* s_shader = 0;
static dy_texture* s_whitetexture = 0;

static dy_vbo s_mesh_vbo;
static dy_ibo s_mesh_ibo;
static unsigned int s_mesh_elements = 0;


float get_time()
{
	return clock() / (float)CLOCKS_PER_SEC;
}

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

		int start = vb->used;
		int hei = 0;
		dy_rhalfedge* first = face->first, * he = first;
		do
		{
			dy_vertex v;
			v.color = dy_color_hsv(plidx / (float)mesh->face_count, 1.0, 1.0);
			v.uv = { 0,0 };
			v.pos = *he->vert->pos;
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


void render(float ff)
{
	int width = 0, height = 0;
	dy_engine_window_size(&width, &height);

	// Bind the shader first!
	dy_shader_bind(s_shader);


	// Camera Matrices
	float time = get_time();
	mat4 model = mat4::yrotation(time * ff) * mat4::xrotation(0.7  * time);
	mat4 view = mat4::identity();
	mat4 proj;
	dy_perspective4x4(&proj, 45, 0.1, 100, height / (float)width);
	view.d = { 0,0,-8,1 };

	dy_shader_set(DY_SHADERPARAM_MODEL, &model);
	dy_shader_set(DY_SHADERPARAM_VIEW, &view);
	dy_shader_set(DY_SHADERPARAM_PROJECTION, &proj);


	// Clear Frame
	dy_render_set_clear_colorf(1.0, 0.5, 0.5);
	dy_render_set_clear_depth(1.0);
	dy_render_clear_frame();

	
	// Draw mesh
	dy_texture_bind(s_whitetexture);
	dy_render_draw_mesh(s_mesh_vbo, s_mesh_ibo, 0, s_mesh_elements);

}


int main()
{

	dy_engine_init();
	
	// Create a blank white texture for drawing textureless objects
	unsigned int pixel = 0xFFFFFFFF;
	s_whitetexture = dy_texture_create(DY_TEXTURE_FORMAT_BGRA8, (unsigned char*)&pixel, 1, 1);

	s_shader = dy_shader_create(s_vertexshader, s_fragmentshader);


	// Create a brush from planes
	dy_bplane planes[] =
	{
		{{ 1, 0, 0}, 1},
		{{ 0, 1, 0}, 1},
		{{ 0, 0, 1}, 1},

		{{-1, 0, 0}, 1},
		{{ 0,-1, 0}, 1},
		{{ 0, 0,-1}, 1},

		{{1.0f/sqrtf(2), 1.0f/sqrtf(2), 0}, 1},
	};

	dy_bsolid s;
	s.planes = &planes[0];
	s.plane_count = sizeof(planes) / sizeof(dy_bplane);

	dy_rmesh mesh = dy_bsolid_mesh(&s);

	// Convert the mesh into something drawable
	dy_vtxbuf* vb;
	dy_idxbuf* ib;
	mesh_drawdata(&mesh, vb, ib);
	dy_rmesh_clear(&mesh);

	s_mesh_vbo = dy_render_create_vbo(vb);
	s_mesh_ibo = dy_render_create_ibo(ib);
	s_mesh_elements = ib->used;
	delete vb;
	delete ib;

	// Create the main window
	dy_window* wnd1 = dy_engine_new_window();
	dy_window* wnd2 = dy_engine_new_window();
	dy_engine_window_show(wnd1);
	dy_engine_window_show(wnd2);

	// Main loop
	while (1)
	{
		dy_engine_window_select(wnd1);
		if (dy_engine_living(wnd1))
		{
			dy_engine_event_pump();
			dy_engine_frame_begin();
			render(1);
			dy_engine_frame_end();
		}

		dy_engine_window_select(wnd2);
		if(dy_engine_living(wnd2))
		{
			dy_engine_event_pump();
			dy_engine_frame_begin();
			render(-1);
			dy_engine_frame_end();
		}
	}
	dy_engine_shutdown();
	return 0;
}