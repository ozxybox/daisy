#include "dy_render.h"
#include <glad/glad.h>
#include <stddef.h>
#include <assert.h>

void dy_render_setviewport(int x, int y, int w, int h)
{
	glViewport(x, y, w, h);
}
void dy_render_set_clear_colorf(float r, float g, float b)
{
	glClearColor(r, g, b, 1.0f);
}
void dy_render_set_clear_color8(unsigned char r, unsigned char g, unsigned char b)
{
	glClearColor(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
}
void dy_render_set_clear_depth(float depth)
{
	glClearDepth(depth);
}
void dy_render_clear_frame()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


// Vertex & Index buffer objects
static dy_vbo dy_create_vbo_()
{
	GLuint vao, vbo;
	
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	return { vao, vbo };
}
static dy_ibo dy_create_ibo_()
{
	GLuint ibo;

	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

	return { ibo };
}

void dy_render_destroy_vbo(dy_vbo vbo)
{
	glDeleteBuffers(1, &vbo.vbo);
	glDeleteVertexArrays(1, &vbo.vao);
}
void dy_render_destroy_ibo(dy_ibo ibo)
{
	glDeleteBuffers(1, &ibo.ibo);
}

// Static vbo & ibo
dy_vbo dy_render_create_vbo(dy_vtxbufview* buf)
{
	dy_vbo o = dy_create_vbo_();
	glBufferData(GL_ARRAY_BUFFER, buf->used * sizeof(dy_vertex), buf->buf, GL_STATIC_DRAW);
	return o;
}
dy_ibo dy_render_create_ibo(dy_idxbufview* buf)
{
	dy_ibo o = dy_create_ibo_();
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * buf->used, buf->buf, GL_STATIC_DRAW);
	return o;
}

// Dynamic vbo & ibo
dy_vbo dy_render_create_vbo_dynamic()
{
	dy_vbo o = dy_create_vbo_();
	glBufferData(GL_ARRAY_BUFFER, 0, 0, GL_DYNAMIC_DRAW);
	return o;
}
dy_ibo dy_render_create_ibo_dynamic()
{
	dy_ibo o = dy_create_ibo_();
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, 0, GL_DYNAMIC_DRAW);
	return o;
}
void dy_render_fill_vbo_dynamic(dy_vbo vbo, dy_vtxbufview* buf)
{
	glBindVertexArray(vbo.vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo.vbo);
	glBufferData(GL_ARRAY_BUFFER, buf->used * sizeof(dy_vertex), buf->buf, GL_DYNAMIC_DRAW);
}
void dy_render_fill_ibo_dynamic(dy_ibo ibo, dy_idxbufview* buf)
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * buf->used, buf->buf, GL_DYNAMIC_DRAW);
}

void dy_render_draw_mesh(dy_vbo vbo, dy_ibo ibo, unsigned int start, unsigned int count)
{
	
	assert(glGetError() == 0);

	glBindVertexArray(vbo.vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo.vbo);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(
		0,                                // position 0
		3,                                // three floats
		GL_FLOAT,                         // elements are floats
		GL_FALSE,                         //
		sizeof(dy_vertex),                // stride
		(void*)offsetof(dy_vertex, pos)   // position within stride
	);
	glVertexAttribPointer(
		1,                                // position 1
		3,                                // three floats
		GL_FLOAT,                         // elements are floats
		GL_FALSE,                         //
		sizeof(dy_vertex),                // stride
		(void*)offsetof(dy_vertex, color) // position within stride
	);
	glVertexAttribPointer(
		2,                                // position 2
		2,                                // two floats
		GL_FLOAT,                         // elements are floats
		GL_FALSE,                         //
		sizeof(dy_vertex),                // stride
		(void*)offsetof(dy_vertex, uv)    // position within stride
	);
	glVertexAttribPointer(
		3,                                // position 3
		3,                                // three floats
		GL_FLOAT,                         // elements are floats
		GL_FALSE,                         //
		sizeof(dy_vertex),                // stride
		(void*)offsetof(dy_vertex, norm)  // position within stride
	);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo.ibo);
	glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_SHORT, (void*)(start * sizeof(unsigned short)));


	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);


	assert(glGetError() == 0);


}

void dy_render_draw_mesh(dy_mesh* mesh)
{
	dy_render_draw_mesh(mesh->vbo, mesh->ibo, 0, mesh->elements);
}


void dy_render_draw_dynamic(dy_vtxbufview* vb, dy_idxbufview* ib)
{
	// FIXME: We need to clean up this resource on shutdown!!!!!!
	static dy_vbo s_dynvb = dy_render_create_vbo_dynamic();
	static dy_ibo s_dynib = dy_render_create_ibo_dynamic();
	static dy_mesh s_dynmesh = { s_dynvb, s_dynib, 0 };

	dy_render_fill_vbo_dynamic(s_dynvb, vb);
	dy_render_fill_ibo_dynamic(s_dynib, ib);
	s_dynmesh.elements = ib->used;

	dy_render_draw_mesh(&s_dynmesh);
}
