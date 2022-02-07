#pragma once

#include "dy_vertex.h"

// Vertex and Index buffer objects
typedef struct { unsigned int vao, vbo; } dy_vbo;
typedef struct { unsigned int ibo; } dy_ibo;


// Drawing functions
void dy_render_set_clear_colorf(float r, float g, float b);
void dy_render_set_clear_color8(unsigned char r, unsigned char g, unsigned char b);
void dy_render_set_clear_depth(float depth);
void dy_render_clear_frame();


// View 
void dy_render_setviewport(int x, int y, int w, int h);


// Vertex & Index buffer objects
void dy_render_destroy_vbo(dy_vbo vbo);
void dy_render_destroy_ibo(dy_ibo ibo);

// Static vbo & ibo
dy_vbo dy_render_create_vbo(dy_vtxbuf* buf);
dy_ibo dy_render_create_ibo(dy_idxbuf* buf); // ONLY CALL AFTER CREATING A VBO!

// Dynamic vbo & ibo
dy_ibo dy_render_create_ibo_dynamic(dy_idxbuf* buf);
dy_vbo dy_render_create_vbo_dynamic(dy_vtxbuf* buf);
void dy_render_fill_vbo_dynamic(dy_vbo vbo, dy_vtxbuf* buf);
void dy_render_fill_ibo_dynamic(dy_ibo ibo, dy_idxbuf* buf);

void dy_render_draw_mesh(dy_vbo vbo, dy_ibo ibo, unsigned int start, unsigned int count);
