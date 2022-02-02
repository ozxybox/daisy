#include "dy_engine.h"
#include "dy_render.h"
#include "dy_shader.h"
#include "dy_texture.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <math.h>

static const char* dy_painter_vertexshader =
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
static const char* dy_painter_fragmentshader =
"#version 330 core\n"
"precision lowp float;"
"in vec3 v_color;"
"in vec2 v_uv;"
"out vec4 o_fragColor;"
"uniform sampler2D u_tex;"
"void main() {"
"    o_fragColor = vec4(v_color.xyz, 1.0) * vec4(texture(u_tex,v_uv).xyz, 1.0);"
"}";


struct dy_window_data
{
	GLFWwindow* window;

	dy_vbo screenvbo;
	dy_ibo screenibo;
};

// Render window
static dy_framebuffer* s_offscreen_framebuffer;
static GLFWwindow* s_hidden_window = 0;

// Current window
static dy_window_data* s_window = 0;

static dy_texture* s_whitetexture = 0;

dy_vbo s_cube_vbo;
dy_ibo s_cube_ibo;
dy_shader* shader;


static dy_vertex s_cube[] =
{
	// Back
	{{  0,  1,  0}, {1,1,0}, {0,0}},
	{{  1,  1,  0}, {1,1,0}, {1,0}},
	{{  1,  0,  0}, {1,1,0}, {1,1}},
	{{  0,  0,  0}, {1,1,0}, {0,1}},

	// Front
	{{  0,  0,  1}, {0,0,1}, {0,0}},
	{{  1,  0,  1}, {0,0,1}, {1,0}},
	{{  1,  1,  1}, {0,0,1}, {1,1}},
	{{  0,  1,  1}, {0,0,1}, {0,1}},

	// Bottom
	{{  0,  0,  0}, {1,0,1}, {0,0}},
	{{  1,  0,  0}, {1,0,1}, {1,0}},
	{{  1,  0,  1}, {1,0,1}, {1,1}},
	{{  0,  0,  1}, {1,0,1}, {0,1}},

	// Top
	{{  0,  1,  1}, {0,1,0}, {0,0}},
	{{  1,  1,  1}, {0,1,0}, {1,0}},
	{{  1,  1,  0}, {0,1,0}, {1,1}},
	{{  0,  1,  0}, {0,1,0}, {0,1}},

	// Left
	{{  0,  0,  1}, {0,1,1}, {0,0}},
	{{  0,  1,  1}, {0,1,1}, {1,0}},
	{{  0,  1,  0}, {0,1,1}, {1,1}},
	{{  0,  0,  0}, {0,1,1}, {0,1}},

	// Right
	{{  1,  0,  0}, {1,0,0}, {0,0}},
	{{  1,  1,  0}, {1,0,0}, {1,0}},
	{{  1,  1,  1}, {1,0,0}, {1,1}},
	{{  1,  0,  1}, {1,0,0}, {0,1}},
};


int dy_engine_init()
{
	if (!glfwInit())
		return 1;


	// Create an invisible window
	// Attach a context
	// use it to create frame buffers
	// render framebuffers to other windows
	// still need to make a quad or tri for drawing the fbo to each screen!

	const int window_width = 640;
	const int window_height = 480;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	s_hidden_window = glfwCreateWindow(640, 480, "", NULL, NULL);
	glfwMakeContextCurrent(s_hidden_window);
	

	// Initialize OpenGL
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		return 1;
	
	// Create the offscreen framebuffer for rendering window content
	dy_texture* color = dy_texture_create(DY_TEXTURE_FORMAT_RGB8, 0, 1024, 1024);
	s_offscreen_framebuffer = dy_framebuffer_create(color, 1);

	// Create a blank white texture for drawing textureless objects
	unsigned int pixel = 0xFFFFFFFF;
	s_whitetexture = dy_texture_create(DY_TEXTURE_FORMAT_BGRA8, (unsigned char*)&pixel, 1, 1);
	

	dy_shader_init();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	shader = dy_shader_create(dy_painter_vertexshader, dy_painter_fragmentshader);

	// Cube Geo
	{
		dy_vtxbuf vb(24);
		vb.push_many(&s_cube[0], 24);

		dy_idxbuf ib(36);
		ib.push_quad(0);
		ib.push_quad(4);
		ib.push_quad(8);
		ib.push_quad(12);
		ib.push_quad(16);
		ib.push_quad(20);

		s_cube_vbo = dy_render_create_vbo(&vb);
		s_cube_ibo = dy_render_create_ibo(&ib);
	}


	return 0;
}


void dy_engine_shutdown()
{
	glfwTerminate();
}


int dy_engine_living(dy_window* window)
{
	return !glfwWindowShouldClose((GLFWwindow*)window);
}
void dy_engine_event_pump()
{
	glfwPollEvents();
}


void dy_engine_render(float f)// int width, int height)
{
	// FIXME: AA
	int width = 0, height = 0;
	glfwGetWindowSize(s_window->window, &width, &height);


	// Camera Matrices
	float time = glfwGetTime() + f;
	mat4 model = mat4::yrotation(time) * mat4::xrotation(0.7 * time);
	mat4 view = mat4::identity();
	mat4 proj;
	dy_perspective4x4(&proj, 45, 0.1, 100, height / (float)width);
	view.d = { 0,0,-8,1 };

	dy_shader_set(DY_SHADERPARAM_MODEL, &model);
	dy_shader_set(DY_SHADERPARAM_VIEW, &view);
	dy_shader_set(DY_SHADERPARAM_PROJECTION, &proj);

	// Clear Frame
	//glClearColor(1.0, 0.5, 0.5, 1.0);
	glClearColor(cos(f) * 0.5 + 0.5, sin(f) * 0.5 + 0.5, sin(cos(f)) * 0.5 + 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Draw mesh
	dy_shader_bind(shader);
	dy_texture_bind(s_whitetexture);
	dy_render_draw_mesh(s_cube_vbo, s_cube_ibo, 0, 36);
}

void dy_engine_frame_begin()
{
	// Switch to our rendering window
	glfwMakeContextCurrent(s_hidden_window);

	// Bind the offscreen framebuffer
	dy_texture* fbtex = dy_framebuffer_color(s_offscreen_framebuffer);
	dy_framebuffer_bind(s_offscreen_framebuffer);
	unsigned int fbtexwidth, fbtexheight;
	dy_texture_get_dimensions(fbtex, &fbtexwidth, &fbtexheight);
	dy_render_setviewport(0, 0, fbtexwidth, fbtexheight);

	// Get the selected window size
	int width = 0, height = 0;
	glfwGetWindowSize(s_window->window, &width, &height);
}


void dy_engine_frame_end()
{
	// Frame is done. Let's push it to our target window
	dy_framebuffer_unbind();

	// FIXME: AA
	int width = 0, height = 0;
	glfwGetWindowSize(s_window->window, &width, &height);

	// Switch back to our selected window
	glfwMakeContextCurrent(s_window->window);

	// Setup the camera to fullscreen the FBO
	dy_render_setviewport(0, 0, width, height);
	mat4 eye = mat4::identity();
	mat4 proj;
	dy_ortho4x4(&proj, 0, 1, 0, 1, -10, 10);
	dy_shader_set(DY_SHADERPARAM_MODEL, &eye);
	dy_shader_set(DY_SHADERPARAM_VIEW, &eye);
	dy_shader_set(DY_SHADERPARAM_PROJECTION, &proj);

	// Draw the screen quad with the fbo bound
	dy_shader_bind(shader);
	dy_texture_bind(dy_framebuffer_color(s_offscreen_framebuffer));
	dy_render_draw_mesh(s_window->screenvbo, s_window->screenibo, 0, 6);

	// Push the frame to the selected window
	glfwSwapBuffers(s_window->window);
}



dy_window* dy_engine_new_window()
{
	const int window_width = 640;
	const int window_height = 480;

	// Create the glfw window
	GLFWwindow* wnd = glfwCreateWindow(window_width, window_height, "Window Title", NULL, s_hidden_window);
	glfwMakeContextCurrent(wnd);

	
	// Create a screen object to render our framebuffer to 
	static dy_vertex s_quad[] =
	{
		{{  0,  0,  0}, {1,1,1}, {0,0}},
		{{  1,  0,  0}, {1,1,1}, {1,0}},
		{{  1,  1,  0}, {1,1,1}, {1,1}},
		{{  0,  1,  0}, {1,1,1}, {0,1}},
	};
	dy_vtxbuf vb(4);
	vb.push_many(&s_quad[0], 4);

	dy_idxbuf ib(6);
	ib.push_quad(0);

	dy_vbo vbo = dy_render_create_vbo(&vb);
	dy_ibo ibo = dy_render_create_ibo(&ib);


	// Record all the data and return it 
	dy_window_data* data = (dy_window_data*)malloc(sizeof(dy_window_data));
	data->window = wnd;
	data->screenvbo = vbo;
	data->screenibo = ibo;

	return (dy_window*)data;
}

void dy_engine_window_select(dy_window* window)
{
	dy_window_data* data = (dy_window_data*)window;
	s_window = data;
}

void dy_engine_window_show(dy_window* window)
{
	dy_window_data* data = (dy_window_data*)window;
	glfwShowWindow(data->window);
}

#define GLFW_EXPOSE_NATIVE_WIN32 1
#include <GLFW/glfw3native.h>

void* dy_engine_hwnd(dy_window* window)
{
	dy_window_data* data = (dy_window_data*)window;
	return glfwGetWin32Window(data->window);
}
