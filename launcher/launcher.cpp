#include "engine/dy_engine.h"
#include "engine/dy_render.h"
#include "engine/dy_texture.h"
#include "engine/dy_shader.h"
#include "csg/dy_csg.h"
#include "csg/dy_brush.h"
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <dy_ustack.h>
#include <assert.h>
#include <X9PClient.h>
#include "launcher.h"

static const char* s_vertexshader =
"#version 330 core\n"
"layout(location = 0) in vec3 a_pos;"
"layout(location = 1) in vec3 a_color;"
"layout(location = 2) in vec2 a_uv;"
"layout(location = 3) in vec3 a_norm;"
"uniform mat4 u_model;"
"uniform mat4 u_view;"
"uniform mat4 u_projection;"
"out vec3 v_color;"
"out vec2 v_uv;"
"out vec3 v_normal;"
"void main(){"
"    v_color=a_color;"
"    v_uv=a_uv;"
"    gl_Position = u_projection * u_view * u_model * vec4(a_pos.xyz, 1.0);"
"    v_normal = normalize(transpose(inverse(mat3(u_model))) * a_norm.xyz);"
"}";
static const char* s_fragmentshader =
"#version 330 core\n"
//"precision lowp float;"
"in vec3 v_color;"
"in vec2 v_uv;"
"in vec3 v_normal;"
"out vec4 o_fragColor;"
"uniform sampler2D u_tex;"
"void main() {"
"    vec3 lightDir = normalize(-vec3(-0.6, -1, 0.2));"
"    float diff = pow(0.5 + 0.5 * dot(v_normal, lightDir), 2.0);"
"    vec3 diffuse = vec3(0.7, 0.7, 0.7) * (diff + 0.3);"
//"    o_fragColor = vec4(abs(v_normal), 1.0);"
"    o_fragColor = vec4(texture(u_tex,v_uv).xyz, 1.0) * vec4(diffuse, 1.0) * vec4(v_color.xyz, 1.0);"
//"    o_fragColor = vec4(v_color.xyz, 1.0) * vec4(texture(u_tex,v_uv).xyz, 1.0);"
"}";

void download_world(const char* ip, const char* port, dy_ustack<dy_bsolid>& solidlist)
{
	SystemInitSocket();
	bool keeppulling = true;
	X9PClient cl;
	cl.Begin(ip, port);
	xhnd root = cl.NewFileHandle(0);

	int waiting;

	waiting = 1;
	cl.Tattach(root, NOFID, XSTR_L("user"), XSTR_L("/"), [&](xerr_t err, qid_t* q) {
		waiting = 0;
		if (err) { printf(err); return; };
		printf("Connected!\n");
	});
	while (waiting) cl.ProcessPackets();

	waiting = 1;
	xhnd vmf = cl.NewFileHandle(0);
	cl.Twalk(root, vmf, 1, XSTR_L("world1.vmf"), [&](xerr_t err, uint16_t nwqid, qid_t* wqid) {
		waiting = 0;
		if (err) { printf(err); return; };
	});
	while (waiting) cl.ProcessPackets();


	// Find all solids
	bool ismore = true;
	uint64_t offset = 0;
	while (ismore)
	{
		waiting = 1;
		cl.Tread(vmf, offset, 1024, [&](xerr_t err, uint32_t count, uint8_t* data) {
			waiting = 0;
			if (count == 0)
			{
				ismore = false;
				return;
			}

			stat_t* s = (stat_t*)data;
#if VERBOSE
			xstrprint(s->name());
			putchar('\n');
#endif
			offset += s->size;

			
			dy_ustack<dy_bplane> planelist;
			
			// Open the solid's directory
			int waiting2 = 1;
			xhnd solid = cl.NewFileHandle(0);
			cl.Twalk(vmf, solid, 1, s->name(), [&](xerr_t err, uint16_t nwqid, qid_t* wqid) {
				waiting2 = 0;
				if (err) printf(err);
			});
			while (waiting2) cl.ProcessPackets();

			// Scan for planes
			bool moreplanes = true;
			uint64_t offset2 = 0;
			while (moreplanes)
			{
				waiting2 = 3;
				cl.Tread(solid, offset2, 1024, [&](xerr_t err, uint32_t count, uint8_t* data) {
					waiting2--;
					if (count == 0)
					{
						waiting2 = 0;
						moreplanes = false;
						return;
					}

					stat_t* s2 = (stat_t*)data;
#if VERBOSE
					putchar('\t');
					xstrprint(s2->name());
					putchar('\n');
#endif
					offset2 += s2->size;

					// Walk to file
					xhnd plane = cl.NewFileHandle(0);
					cl.Twalk(solid, plane, 1, s2->name(), [&](xerr_t err, uint16_t nwqid, qid_t* wqid) {
						waiting2--;
						if (err) printf(err);
					});
					
					// FIXME: Open the file!
					
					// Read data from plane
					cl.Tread(plane, 0, sizeof(dy_bplane), [&](xerr_t err, uint32_t count, uint8_t* buf) {
						waiting2--;
						if (err) printf(err);
						assert(count == sizeof(dy_bplane));
						
						planelist.push(reinterpret_cast<dy_bplane*>(buf));
					});
				});
				while (waiting2) cl.ProcessPackets();

			}

			// Let go of the folder
			cl.ReleaseFileHandle(0, solid);


			// Push this new solid to our list
			dy_bsolid brush = { planelist.packed(), planelist.count };
			solidlist.push(&brush);
		});
		while (waiting) cl.ProcessPackets();
	}

	cl.End();

	printf("Downloaded %d solids!\n", solidlist.count);
}


static dy_shader* s_shader = 0;
static dy_texture* s_whitetexture = 0;

dy_camera s_camera = { { -4562.83594, 156.550415, 5950.64111}, 0, 0 };
//static float s_camera_yaw = 0;
//static float s_camera_pitch = 0;
//static vec3  s_camera_pos = { -3000, 200, 2000 };
//vec3 s_camera_pos = { -4562.83594, 156.550415, 5950.64111 };

struct brush_draw
{
	dy_vbo vbo;
	dy_ibo ibo;
	unsigned int elements = 0;
};

static dy_ustack<brush_draw> s_brush_draws;

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

			//float diff = 0.5 + 0.5 * pow(0.5 + 0.5 * vec3::dot(face->plane->norm, { 1 / sqrtf(2), 1 / sqrtf(2), 0 }), 2.0);
			v.color = {1.0, 1.0, 1.0};//dy_color_hsv(plidx / (float)mesh->face_count, 0.6, 1.0);
			v.uv = { 0,0 };
			v.pos = *he->vert->pos;
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

void fill_world(const char* ip, const char* port)
{
	dy_ustack<dy_bsolid> solids;
	download_world(ip, port, solids);

	for (dy_bsolid* solid : solids)
	{
		dy_rmesh mesh = dy_bsolid_mesh(solid);

		// Convert the mesh into something drawable
		dy_vtxbuf* vb;
		dy_idxbuf* ib;
		mesh_drawdata(&mesh, vb, ib);
		dy_rmesh_clear(&mesh);

		brush_draw bd;
		bd.vbo = dy_render_create_vbo(vb);
		bd.ibo = dy_render_create_ibo(ib);
		bd.elements = ib->used;
		delete vb;
		delete ib;

		s_brush_draws.push(&bd);
	}

}

void render()
{
	int width = 0, height = 0;
	dy_engine_window_size(&width, &height);

	// Bind the shader first!
	dy_shader_bind(s_shader);


	// Camera Matrices
	float time = get_time();
	mat4 model = mat4::identity();
	mat4 view = mat4::identity();
	view.d = { 0, 0, 0, 1 };
	mat4 hh = mat4::identity();
	hh.d = { -s_camera.pos.x, -s_camera.pos.y, -s_camera.pos.z, 1 };
	view = mat4::xrotation(-s_camera.pitch) * mat4::yrotation(-s_camera.yaw) * hh;
	mat4 proj;
	dy_perspective4x4(&proj, 65, 0.1, 90000, height / (float)width);

	dy_shader_set(DY_SHADERPARAM_MODEL, &model);
	dy_shader_set(DY_SHADERPARAM_VIEW, &view);
	dy_shader_set(DY_SHADERPARAM_PROJECTION, &proj);


	// Clear Frame
	//dy_render_set_clear_colorf(1.0, 0.5, 0.5);
	dy_render_set_clear_colorf(1.0, 0.9, 0.9);
	dy_render_set_clear_depth(1.0);
	dy_render_clear_frame();

	
	// Draw mesh
	dy_texture_bind(s_whitetexture);
	for(brush_draw* bd : s_brush_draws)
		dy_render_draw_mesh(bd->vbo, bd->ibo, 0, bd->elements);

}

extern void api_update();
extern void api_init();

int main(int argc, const char** args)
{

	dy_engine_init();

	
	// Create a blank white texture for drawing textureless objects
	unsigned int pixel = 0xFFFFFFFF;
	s_whitetexture = dy_texture_create(DY_TEXTURE_FORMAT_BGRA8, (unsigned char*)&pixel, 1, 1);

	s_shader = dy_shader_create(s_vertexshader, s_fragmentshader);

	fill_world(args[1], args[2]);

	api_init();

	// Create the main window
	dy_window* wnd = dy_engine_new_window();
	dy_engine_window_show(wnd);

	float lastframetime = get_time();

	double smx = 0, smy = 0;
	// Main loop
	while (dy_engine_living(wnd))
	{
		float curtime = get_time();
		float dt = curtime - lastframetime;
		lastframetime = curtime;

		dy_engine_window_select(wnd);
		dy_engine_event_pump();
		dy_engine_frame_begin();
		render();
		dy_engine_frame_end();

		static bool begin_camera = false;
		if (dy_engine_key_down(32))
		{
			static double lmx = 0, lmy = 0;
			double mx = 0, my = 0;
			dy_engine_mouse_pos(&mx, &my);
			if (!begin_camera)
			{
				lmx = mx;
				lmy = my;
			}

			double dmx = mx - lmx;
			double dmy = my - lmy;
			lmx = mx;
			lmy = my;

			//s_camera.pitch += (dmy - 320) / 640.0f * (DY_PI * 2);
			//s_camera.yaw   += (dmx - 240) / 240.0f * (DY_PI * 0.5);
			s_camera.pitch += dmy / 640.0f * (DY_PI * 2);
			s_camera.yaw += dmx / 240.0f * (DY_PI * 0.5);


			if (s_camera.pitch > DY_PI * 0.5) s_camera.pitch = DY_PI * 0.5;
			if (s_camera.pitch < -DY_PI * 0.5) s_camera.pitch = -DY_PI * 0.5;

			begin_camera = true;
		}
		else
			begin_camera = false;

		vec4 mv = {0,0,0,1};
		if (dy_engine_key_down(87)) // W
			mv.z -= 1;
		if (dy_engine_key_down(83)) // S
			mv.z += 1;
		if (dy_engine_key_down(65)) // A
			mv.x -= 1;
		if (dy_engine_key_down(68)) // D
			mv.x += 1;
		mv = 1024 * dt * mv;
		mv = mat4::yrotation(s_camera.yaw) * mat4::xrotation(s_camera.pitch) * mv;
		s_camera.pos += vec3{mv.x, mv.y, mv.z};

		api_update();
	}
	dy_engine_shutdown();
	return 0;
}