#pragma once

// Texture formats
enum DY_TEXTURE_FORMAT
{
	// Depth, Stencil, Component size
	DY_TEXTURE_FORMAT_D8,
	DY_TEXTURE_FORMAT_DS8,

	// Red, Green, Blue, Alpha, Component size
	DY_TEXTURE_FORMAT_RGBA8,
	DY_TEXTURE_FORMAT_RGB8,
	DY_TEXTURE_FORMAT_RG8,
	DY_TEXTURE_FORMAT_R8,
	DY_TEXTURE_FORMAT_BGRA8,
	DY_TEXTURE_FORMAT_BGR8,

};


// Textures //
typedef void dy_texture;

// Pixels can be NULL for a blank texture
dy_texture* dy_texture_create(enum DY_TEXTURE_FORMAT format, unsigned char* pixels, unsigned int width, unsigned int height);
void dy_texture_destroy(dy_texture* texture);
void dy_texture_bind(dy_texture* texture);
void dy_texture_get_dimensions(dy_texture* texture, unsigned int* width, unsigned int* height);


// Framebuffers //
typedef void dy_framebuffer;

dy_framebuffer* dy_framebuffer_create(dy_texture* colortexture, int hasdepth);
// Does not destroy the attached color texture!!
void dy_framebuffer_destroy(dy_framebuffer* framebuffer);
dy_texture* dy_framebuffer_color(dy_framebuffer* framebuffer);
void dy_framebuffer_bind(dy_framebuffer* framebuffer);
void dy_framebuffer_unbind();