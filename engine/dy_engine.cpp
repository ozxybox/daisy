#include "dy_engine.h"
#include "dy_render.h"
#include "dy_shader.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

static const char* dy_painter_vertexshader =
"#version 330 core\n"
"layout(location = 0) in vec3 a_pos;"
"layout(location = 1) in vec3 a_color;"
"layout(location = 2) in vec2 a_uv;"
"uniform mat4 u_model;"
"uniform mat4 u_view;"
"uniform mat4 u_projection;"
"out vec3 v_color;"
"out vec4 v_worldpos;"
"void main(){"
"    v_color=a_color;"
"    gl_Position = u_projection * u_view * u_model * vec4(a_pos.xyz, 1.0);"
"}";
static const char* dy_painter_fragmentshader =
"#version 330 core\n"
"precision lowp float;"
"in vec3 v_color;"
"out vec4 o_fragColor;"
"void main() {"
"    o_fragColor = vec4(v_color.xyz, 1.0);"
"}";

static GLFWwindow* s_window = 0;

dy_vbo vbo;
dy_ibo ibo;
dy_shader* shader;

static dy_vertex s_cube[] =
{
	// Back
	{{  0,  1,  0}, {1,1,0}, {0,0}},
	{{  1,  1,  0}, {1,1,0}, {0,0}},
	{{  1,  0,  0}, {1,1,0}, {0,0}},
	{{  0,  0,  0}, {1,1,0}, {0,0}},

	// Front
	{{  0,  0,  1}, {0,0,1}, {0,0}},
	{{  1,  0,  1}, {0,0,1}, {0,0}},
	{{  1,  1,  1}, {0,0,1}, {0,0}},
	{{  0,  1,  1}, {0,0,1}, {0,0}},

	// Bottom
	{{  0,  0,  0}, {1,0,1}, {0,0}},
	{{  1,  0,  0}, {1,0,1}, {0,0}},
	{{  1,  0,  1}, {1,0,1}, {0,0}},
	{{  0,  0,  1}, {1,0,1}, {0,0}},

	// Top
	{{  0,  1,  1}, {0,1,0}, {0,0}},
	{{  1,  1,  1}, {0,1,0}, {0,0}},
	{{  1,  1,  0}, {0,1,0}, {0,0}},
	{{  0,  1,  0}, {0,1,0}, {0,0}},

	// Left
	{{  0,  0,  1}, {0,1,1}, {0,0}},
	{{  0,  1,  1}, {0,1,1}, {0,0}},
	{{  0,  1,  0}, {0,1,1}, {0,0}},
	{{  0,  0,  0}, {0,1,1}, {0,0}},

	// Right
	{{  1,  0,  0}, {1,0,0}, {0,0}},
	{{  1,  1,  0}, {1,0,0}, {0,0}},
	{{  1,  1,  1}, {1,0,0}, {0,0}},
	{{  1,  0,  1}, {1,0,0}, {0,0}},
};

int engine_init()
{
	if (!glfwInit())
		return 1;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);

	const int window_width = 640;
	const int window_height = 480;

	s_window = glfwCreateWindow(640, 480, "Window Title", NULL, NULL);
	glfwMakeContextCurrent(s_window);
	if (!s_window)
		return 1;

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		return 1;

	dy_shader_init();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	

	

	dy_vtxbuf vb(24);
	vb.push_many(&s_cube[0], 24);
	
	dy_idxbuf ib(36);
	ib.push_quad(0);
	ib.push_quad(4);
	ib.push_quad(8);
	ib.push_quad(12);
	ib.push_quad(16);
	ib.push_quad(20);

	vbo = dy_render_create_vbo(&vb);
	ibo = dy_render_create_ibo(&ib);

	shader = dy_shader_create(dy_painter_vertexshader, dy_painter_fragmentshader);

	return 0;
}

void engine_shutdown()
{
	glfwTerminate();
}


int engine_living()
{
	return !glfwWindowShouldClose(s_window);
}
void engine_frame_pump()
{
	glfwPollEvents();

	int width = 0, height = 0;
	glfwGetWindowSize(s_window, &width, &height);

	dy_render_setviewport(0, 0, width, height);

	mat4 model = mat4::yrotation(glfwGetTime()) * mat4::xrotation(0.7 * glfwGetTime());
	mat4 view  = mat4::identity();
	mat4 proj;
	view.d = {0,0,-8,1};
	//dy_ortho4x4(&proj, 0, width, 0, height, -100, 100);
	dy_perspective4x4(&proj, 45, 0.1, 100, height / (float)width);

	dy_shader_set(DY_SHADERPARAM_MODEL, &model);
	dy_shader_set(DY_SHADERPARAM_VIEW, &view);
	dy_shader_set(DY_SHADERPARAM_PROJECTION, &proj);

	glClearColor(1.0, 0.5, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	dy_shader_bind(shader);
	dy_render_draw_mesh(vbo, ibo, 0, 36);

	glfwSwapBuffers(s_window);
}