#pragma once
#include "dy_render.h"
#include "dy_shader.h"
#include "dy_texture.h"

/*
struct dy_material
{
	dy_shader* shader;
	dy_texture* tex0;
	dy_texture* tex1;
};
*/

struct dy_model
{
	dy_mesh* meshes;
	unsigned int count;
};


void dy_asset_init();
void dy_asset_shutdown();


dy_model* dy_asset_model_load(const char* path);
void dy_asset_model_draw(dy_model* model);


// TODO: replace with materials?
dy_texture* dy_asset_texture_load(const char* path);
