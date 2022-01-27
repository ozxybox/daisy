#include "engine.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

static GLFWwindow* s_window = 0;

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

	glEnable(GL_DEPTH_TEST);
	
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

	glClearColor(1.0, 0.5, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glfwSwapBuffers(s_window);
}