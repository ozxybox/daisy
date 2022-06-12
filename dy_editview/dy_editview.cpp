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
#include "dy_editor.h"
#include "dy_asset.h"
#include "worldrender.h"
#include "dy_doodle.h"
#include "dy_editsys.h"
#include "dy_debugtext.h"
#include "userpawn.h"
#include "dy_ot.h"
#include <XIO.h>
#include <thread>
#include <chrono>

static const char* s_vertexshader =
"#version 330 core\n"
"layout(location = 0) in vec3 a_pos;"
"layout(location = 1) in vec3 a_color;"
"layout(location = 2) in vec2 a_uv;"
"layout(location = 3) in vec3 a_norm;"
"uniform mat4 u_model;"
"uniform mat4 u_mvp;"
"uniform mat4 u_view;"
"uniform mat4 u_projection;"
"uniform vec4 u_color;"
"uniform vec4 u_texoff;"
"out vec4 v_color;"
"out vec2 v_uv;"
"out vec3 v_normal;"
"out vec4 v_camlocalpos;"
"void main(){"
"    v_color=vec4(a_color,1.0)*u_color;"
"    v_uv= a_uv * u_texoff.zw + u_texoff.xy;"
"    v_camlocalpos = u_view * u_model * vec4(a_pos.xyz, 1.0);"
"    gl_Position = u_mvp * u_model * vec4(a_pos.xyz, 1.0);"
"    v_normal = normalize(transpose(inverse(mat3(u_model))) * a_norm.xyz);"
"}";

static const char* s_fragmentshader =
"#version 330 core\n"
//"precision lowp float;"
"in vec4 gl_FragCoord;"
"in vec4 v_color;"
"in vec2 v_uv;"
"in vec3 v_normal;"
"in vec4 v_camlocalpos;"
"out vec4 o_fragColor;"
"uniform sampler2D u_texcolor;"
"uniform sampler2D u_texmask;"
"uniform vec2      u_resolution;"

"float g_opacityLimit[] = "
"{0.0000, 0.5000, 0.1250, 0.6250,"
" 0.7500, 0.2500, 0.8750, 0.3750,"
" 0.1875, 0.6875, 0.0625, 0.5625,"
" 0.9375, 0.4375, 0.8125, 0.3125};"

"float g_pixelScale = 4.0;"
"float g_seethroughRange = pow(64.0,2.0);"
"float g_ambient = 0.3;"

"void main() {"
"    "
"    float alpha = dot(v_camlocalpos.xyz, v_camlocalpos.xyz) / g_seethroughRange;" // We cant bulk our stuff into 4x4 blocks and this makes the dither look off!
"    float pixelScale = g_pixelScale;"
"    ivec2 dp = ivec2(floor(gl_FragCoord.xy / pixelScale )) % 4;"
"    int index = dp.y * 4 + dp.x;"

"    vec3 lightDir = normalize(-vec3(-0.6, -1, 0.2));"
"    float diff = pow(0.5 + 0.5 * dot(v_normal, lightDir), 2.0);"
"    diff = diff * (1.0 - g_ambient) + g_ambient;"

"    float coloring = texture(u_texmask,v_uv).x;"
"    float lighting = texture(u_texmask,v_uv).z;"

"    vec4 texcol = texture(u_texcolor, v_uv);"
"    vec4 color  = texcol * (v_color * coloring + (1.0 - coloring) * vec4(1.0, 1.0, 1.0, 1.0));"

"    alpha = clamp(alpha, 0.0, 1.0) * texcol.w * v_color.w; " // Calc alpha without the mask!
"    if(alpha <= 0.0 || alpha <= g_opacityLimit[index])"
"        discard;"

"    o_fragColor = vec4(color.xyz * (1.0 + lighting * (diff - 1.0)), 1.0);"
"}";



static dy_shader*  s_shader       = 0;
static dy_texture* s_whitetexture = 0;
static dy_texture* s_blacktexture = 0;
static dy_texture* s_gridtexture  = 0;
dy_texture* s_redtexture   = 0;


//dy_camera s_camera = { {-37.2310944, 151.803650, 72.3481522 }, 0, 0 };
//dy_camera s_camera = { {0,160,160 }, 3.14 * 0.25, 0 };

struct
{
	vec2 screensize;
	mat4 view;
	mat4 proj;
	mat4 mvp;
	dy_frustrum frustrum;
	bool dirty = true;
} s_camData;
dy_camera s_camera = { {0,64,256 }, 0, 0 };
//dy_camera s_camera = { { -635.470337, 13.7063503, -273.269653 }, { 0.0785398632, -2.01585507, 0.00000000 } };

void dy_editor_update_camera()
{
	s_camData.dirty = true;
}


void dy_editor_process_camera()
{

	int width = 0, height = 0;
	dy_engine_window_size(&width, &height);

	// Did our window get resized?
	if (s_camData.screensize.x != width || s_camData.screensize.y != height)
	{
		s_camData.screensize.x = width;
		s_camData.screensize.y = height;
		s_camData.dirty = true;
	}

	// Has our camera been changed?
	if (s_camData.dirty)
	{
		s_camData.dirty = false;

		// Camera Matrices

		mat4 view = s_camera.matrix();
		mat4 proj;

		float fov = 65;
		float near = 0.1;
		float far = 1024 * 8;
		float aspect = s_camData.screensize.y / (float)s_camData.screensize.x;
		dy_perspective4x4(&proj, fov, near, far, aspect);

		s_camData.view = view;
		s_camData.proj = proj;
		s_camData.mvp = proj * view;

		/////////////////////////////////////////////////////////////////////////////////
		float rfov = (fov / 2.0) * (DY_PI / 180.0);
		float G = atanf(tanf(rfov) / aspect);

	#if 1
		mat3 H = mat3::yrotation(s_camera.rotation.yaw) * mat3::xrotation(s_camera.rotation.pitch);
		vec3 pos = s_camera.origin;// { 0, 0, 0 };// { s_camera.pos.x, s_camera.pos.y, s_camera.pos.z };
	#else
		mat3 H = mat3::yrotation(dy_curtime()) * mat3::xrotation(0);
		vec3 pos = { 0, 0, 0 };// { s_camera.pos.x, s_camera.pos.y, s_camera.pos.z };
	#endif

		s_camData.frustrum.near.norm   = H * vec3{ 0, 0,  1 };
		s_camData.frustrum.far.norm    = H * vec3{ 0, 0, -1 };

		s_camData.frustrum.left.norm   = H * vec3{-cosf( G), 0.0, sinf( G)};
		s_camData.frustrum.right.norm  = H * vec3{ cosf( G), 0.0, sinf( G)};

		s_camData.frustrum.top.norm    = H * vec3{ 0.0,  cosf(rfov), sinf(rfov) };
		s_camData.frustrum.bottom.norm = H * vec3{ 0.0, -cosf(rfov), sinf(rfov) };

		s_camData.frustrum.near.d   = s_camData.frustrum.near.norm.dot(pos) - near;
		s_camData.frustrum.far.d    = s_camData.frustrum.far.norm.dot(pos) + far;
		s_camData.frustrum.left.d   = s_camData.frustrum.left.norm.dot(pos);
		s_camData.frustrum.right.d  = s_camData.frustrum.right.norm.dot(pos);
		s_camData.frustrum.top.d    = s_camData.frustrum.top.norm.dot(pos);
		s_camData.frustrum.bottom.d = s_camData.frustrum.bottom.norm.dot(pos);
		////////////////////////////////////////////////////////////////////////////////
	}
}

dy_model* s_binoculars;
dy_texture* s_cam_color;
dy_texture* s_cam_mask;
#include <cmath>
void dy_editor_render()
{
	

	// Bind the shader first!
	dy_shader_bind(s_shader);


	mat4 model = mat4::identity();
	dy_shader_set(DY_SHADERPARAM_MODEL, &model);

	
	// Push the camera data
	dy_shader_set(DY_SHADERPARAM_VIEW,       &s_camData.view);
	dy_shader_set(DY_SHADERPARAM_PROJECTION, &s_camData.proj);
	dy_shader_set(DY_SHADERPARAM_MVP,        &s_camData.mvp);
	dy_shader_set(DY_SHADERPARAM_RESOLUTION, &s_camData.screensize);

	

	// Clear Frame
	dy_render_set_clear_colorf(1.0, 0.9, 0.9);
	dy_render_set_clear_depth(1.0);
	dy_render_clear_frame();

	vec4 texoffset = { 0,0, 1,1 };
	dy_shader_set(DY_SHADERPARAM_TEXOFF, &texoffset);


	int tc = 0;
	int tm = 1;
	dy_shader_set(DY_SHADERPARAM_TEXCOLOR, &tc);
	dy_shader_set(DY_SHADERPARAM_TEXMASK, &tm);

	dy_texture_bind(s_gridtexture, 0);
	dy_texture_bind(s_whitetexture, 1);
	vec4 brushcol = { 1,1,1,1 };
	dy_shader_set(DY_SHADERPARAM_COLOR, &brushcol);
	draw_world(s_camData.frustrum);


	CUserPawn::DrawAll();



	// Draw doodles
	dy_texture_bind(s_whitetexture, 0);
	dy_texture_bind(s_redtexture, 1);

	dy_doodle_render();



}


// TODO:
//   Each binocular eye is a sphere projected onto a plane
//   Take the mouse cursor's ray and rotate the "eyes" to look at the intersection point
//   Project the rotated "eye" onto the "glass" plane
//   Maybe draw it with a custom shader so we can squish and warp the iris

extern XFile s_root;
XFile s_timeline;
void dy_editor_update()
{
	// If we have any updates to our camera, push em out
	dy_editor_process_camera();

	// Convert mouse screen space coordinates to world space coordinates
	double mx, my;
	dy_engine_mouse_pos(&mx, &my);
	mx = mx / s_camData.screensize.x * 2 - 1;
	my = -(my / s_camData.screensize.y * 2 - 1);

	vec4 dn = {(float) mx,(float) my, 1, 1};
	dn = dy_unproject4x4(&s_camData.proj, dn);
	dn.w = 1;
	dn.z = -1;
	dn = mat4::yrotation(s_camera.rotation.yaw) * mat4::xrotation(s_camera.rotation.pitch) * dn;
	dn *= 4096;
	vec3 e = s_camera.origin + vec3{dn.x, dn.y, dn.z};

#if 1
	// Cast the mouse's ray into the world
	dy_ray ray{ s_camera.origin, e };

	static bool acting = false;
	static vec3 startIntersect = {};
	static dy_netdb_objref<dy_brush>  brush = 0;
	static dy_netdb_objref<dy_bplane> plane = 0;
	static XFile op = {};

	if (acting)
	{
		if (dy_engine_mouse_down(0))
		{
			vec3 forward = dy_vec_forward(s_camera.rotation);
			// We create a plane perpendicular to the face's plane pointing the same way as the forward
			vec3 n = plane->norm.cross(plane->norm.cross(forward));
			float d = startIntersect.dot(n);
			dy_bplane mouseplane = { n, d };


			if (raytest_plane(&mouseplane, &ray))	
			{
				dy_doodle_line(ray.intersect, ray.intersect + mouseplane.norm * 64, { 1,1,1,1 });
				dy_doodle_sphere(ray.intersect, 8, { 0,1,0,1.0 });

				float bpd = plane->norm.dot(ray.intersect);
				plane->d = bpd;// 32.0 * round(bpd / 32.0);

				dy_rmesh_clear(&brush->mesh);
				dy_render_destroy_ibo(brush->draw->ibo);
				dy_render_destroy_vbo(brush->draw->vbo);
				delete brush->draw;

				brush_remesh(brush);

				// Update 

				dy_ot_data_update_plane up;
				up.target = plane.id();
				up.content = *plane.ref();
				op.Write(0, sizeof(dy_ot_data_update_plane), &up);

			}
			else
				printf("AA\n");
		
		}
		else
		{
			acting = false;
			plane = 0;
			// Complete
			op.Clunk();
			printf("done\n");
		}

	}
	else
	{
		// Begin selection
		raytest(&ray);
		if (ray.t != FLT_MAX)
		{
			if (ray.face)
			{
				dy_doodle_rface(ray.face, {1,0,0,0.5});
			}

			//dy_doodle_line(s_camera.pos, ray.poi, vec4{ 0,0,1, 1.0 }, 2);
			dy_doodle_sphere(ray.intersect, 8, { 0,1,0,1.0 });

			if (dy_engine_mouse_down(0))
			{
				acting = true;
				startIntersect = ray.intersect;
				plane = ray.face->plane;
				brush = ray.brush;
				printf("%d\n", plane.id());

				s_timeline.Walk(op, "");
				op.Create(XSTRL("update_plane"), X9P_DM_PERM_MASK, X9P_OPEN_EXEC);
				
			}
		}

	}
#endif
	
	dy_editsys_client_update();
}

void dy_editor_init(int argc, const char** args)
{
	dy_asset_init();

	// Create a blank white texture for drawing textureless objects
	unsigned int whitepixel = 0xFFFFFFFF;
	s_whitetexture = dy_texture_create(DY_TEXTURE_FORMAT_BGRA8, (unsigned char*)&whitepixel, 1, 1);
	unsigned int blackpixel = 0xFF000000;
	s_blacktexture = dy_texture_create(DY_TEXTURE_FORMAT_BGRA8, (unsigned char*)&blackpixel, 1, 1);
	unsigned int redpixel = 0xFFFF0000;
	s_redtexture = dy_texture_create(DY_TEXTURE_FORMAT_BGRA8, (unsigned char*)&redpixel, 1, 1);


	s_gridtexture = dy_asset_texture_load("C:/Dev/Calcifer/out/Debug/assets/grid.png");//dy_texture_create(DY_TEXTURE_FORMAT_BGRA8, (unsigned char*)&gridpixels[0], 2, 2);

	s_shader = dy_shader_create(s_vertexshader, s_fragmentshader);

	
	dy_engine_update_time();
	dy_doodle_init();
	dy_debugtext_init();

	dy_editsys_begin_client("127.0.0.1", "27015", "user");

	CUserPawn* pawn;

	/*
	pawn = new CUserPawn();
	pawn->m_username = XSTRL("joe");
	pawn->m_color = {0.5,1,0.5};
	pawn->m_transform.origin = { 0, 0, 0 };
	*/

	pawn = new CUserPawn();
	pawn->m_username = XSTRL("One");
	pawn->m_color = { 1,0.5,0.5};
	pawn->m_transform.origin = { 128, 0, 0 };
	pawn->m_transform.rotation = { DY_PI * 0.125, DY_PI * 1.25, 0 };


	pawn = new CUserPawn();
	pawn->m_username = XSTRL("Two");
	pawn->m_color = { 0.5, 1, 0.5 };
	pawn->m_transform.origin = { -128, 0, 0 };
	pawn->m_transform.rotation = { -DY_PI * 0.125, -DY_PI * 1.25, 0 };

	s_root.Walk(s_timeline, "timeline");
	s_root.Await();
}
// write tstat to dir