#pragma once

typedef void dy_shader;
enum 
{
    DY_SHADERPARAM_MODEL,
    DY_SHADERPARAM_VIEW,
    DY_SHADERPARAM_PROJECTION,

    DY_SHADERPARAM_COUNT
};


dy_shader* dy_shader_create(const char* vs, const char* fs);
void dy_shader_destroy(dy_shader* shader);

void dy_shader_bind(dy_shader* shader);
void dy_shader_set(int uniform, void* data);

void dy_shader_init();