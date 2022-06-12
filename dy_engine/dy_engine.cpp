#include "dy_engine.h"
#include "dy_render.h"
#include "dy_shader.h"
#include "dy_texture.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <assert.h>

static const char* s_window_vertexshader =
"#version 330 core\n"
"uniform mat4 u_projection;"
"out vec2 v_uv;"
"void main(){"
"    int id = gl_VertexID;"
"    vec2 pos = vec2(id & 1, id >> 1 );"
"    v_uv=pos;"
"    gl_Position = u_projection * vec4(pos, 0.0, 1.0);"
"}";
static const char* s_window_fragmentshader =
"#version 330 core\n"
"in vec2 v_uv;"
"out vec4 o_fragColor;"
"uniform sampler2D u_tex;"
"void main() {"
"    o_fragColor = vec4(texture(u_tex,v_uv).xyz, 1.0);"
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

// Shader for rendering the FBO to the window
static dy_shader* s_windowshader;


// Return the time of the current frame
static float s_currentTime = 0.0;
float dy_curtime()
{
	return s_currentTime;
}
float dy_engine_update_time()
{
	s_currentTime = glfwGetTime();
	return s_currentTime;
}
float dy_realtime()
{
	return glfwGetTime();
}

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
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	//glfwWindowHint(GLFW_REFRESH_RATE, 30.0);

	s_hidden_window = glfwCreateWindow(window_width, window_height, "", NULL, NULL);
	glfwMakeContextCurrent(s_hidden_window);
	

	// Initialize OpenGL
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		return 1;
	
	// Hidden window doesn't need to be limited.
	// VSYNC ON
	//	glfwSwapInterval(1);

	// Create the offscreen framebuffer for rendering window content
	dy_texture* color = dy_texture_create(DY_TEXTURE_FORMAT_RGB32F, 0, 2048, 2048);
	s_offscreen_framebuffer = dy_framebuffer_create(color, 1);


	dy_shader_init();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	s_windowshader = dy_shader_create(s_window_vertexshader, s_window_fragmentshader);

	return 0;
}


void dy_engine_shutdown()
{
	glfwTerminate();
}


int dy_engine_living(dy_window* window)
{
	dy_window_data* data = (dy_window_data*)window;
	return !glfwWindowShouldClose(data->window);
}
void dy_engine_event_pump()
{
	glfwPollEvents();
}




void dy_engine_frame_begin()
{

	// Switch to our rendering window so we can draw our scene
	glfwMakeContextCurrent(s_hidden_window);

	// Bind the offscreen framebuffer
	dy_texture* fbtex = dy_framebuffer_color(s_offscreen_framebuffer);
	dy_framebuffer_bind(s_offscreen_framebuffer);

#if 0
	unsigned int fbtexwidth, fbtexheight;
	dy_texture_get_dimensions(fbtex, &fbtexwidth, &fbtexheight);
	dy_render_setviewport(0, 0, fbtexwidth, fbtexheight);
#else
	// FIXME: AA
	int width = 0, height = 0;
	glfwGetWindowSize(s_window->window, &width, &height);
	dy_render_setviewport(0, 0, width, height);
#endif

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

	dy_texture* color = dy_framebuffer_color(s_offscreen_framebuffer);

	unsigned int w = 0, h = 0;
	dy_texture_get_dimensions(color, &w, &h);

	// Bind the shader first!
	dy_shader_bind(s_windowshader);

	// Setup the camera to fullscreen the FBO
	dy_render_setviewport(0, 0, width, height);
	mat4 proj;
	dy_ortho4x4(&proj, 0, width / (float)w, height / (float)h, 0, -10, 10);
	dy_shader_set(DY_SHADERPARAM_PROJECTION, &proj);

	
	// Draw the screen quad with the fbo bound
	dy_texture_bind(color);
	dy_render_draw_mesh(s_window->screenvbo, s_window->screenibo, 0, 6);

	// Push the frame to the selected window
	glfwSwapBuffers(s_window->window);

	// Return to our hidden window
	glfwMakeContextCurrent(s_hidden_window);

}



dy_window* dy_engine_new_window()
{
	const int window_width = 640*2;
	const int window_height = 480*1.5;

	// Create the glfw window
	GLFWwindow* wnd = glfwCreateWindow(window_width, window_height, "Window Title", NULL, s_hidden_window);
	
	// The window needs to own its resources
	glfwMakeContextCurrent(wnd);

	// VSYNC ON
	glfwSwapInterval(2);
	
	// Create a screen object to render our framebuffer to 
	// We're ignoring the vertex buffer. We can specify the screen with the ibo alone
	unsigned short idx[] = {
		0b00, 0b01, 0b11,
		0b00, 0b11, 0b10
	};
	dy_idxbuf ib(6);
	ib.push_many(&idx[0], 6);

	// VBO left blank, but needed for drawing
	dy_vtxbuf vb(0);

	dy_vbo vbo = dy_render_create_vbo(&vb);
	dy_ibo ibo = dy_render_create_ibo(&ib);


	// Record all the data and return it 
	dy_window_data* data = (dy_window_data*)malloc(sizeof(dy_window_data));
	data->window = wnd;
	data->screenvbo = vbo;
	data->screenibo = ibo;

	// Return to the hidden window
	glfwMakeContextCurrent(s_hidden_window);

	return (dy_window*)data;
}

void dy_engine_window_size(int* w, int* h)
{
	glfwGetWindowSize(s_window->window, w, h);
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

int dy_engine_key_down(int key)
{
	return glfwGetKey(s_window->window, key) == GLFW_PRESS;
}
int dy_engine_mouse_down(int button)
{
	return glfwGetMouseButton(s_window->window, button) == GLFW_PRESS;
}

void dy_engine_mouse_pos(double* mx, double* my)
{
	glfwGetCursorPos(s_window->window, mx, my);
}

#define GLFW_EXPOSE_NATIVE_WIN32 1
#include <GLFW/glfw3native.h>

void* dy_engine_hwnd(dy_window* window)
{
	dy_window_data* data = (dy_window_data*)window;
	return glfwGetWin32Window(data->window);
}
