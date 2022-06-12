#pragma once

#include "dy_math.h"

void dy_editsys_begin_client(const char* ip, const char* port, const char* username);
void dy_editsys_client_update();
void dy_editsys_begin_server(const char* port);


struct dy_edituser_info
{
	vec3 color;
	dy_camera camera;
};
