#include "dy_texture.h"
#include <stdlib.h>
#include <cassert>
#include <glad/glad.h>

typedef struct dy_texture_data
{
    GLuint id;

    unsigned int width;
    unsigned int height;

} dy_texture_data;

typedef struct dy_framebuffer_data
{
    GLuint framebuffer;
    GLuint renderbuffer;
    char hasrenderbuffer; // Would be nice to axe this
    
    dy_texture_data* color;

} dy_framebuffer_data;


// Maps to DY_TEXTURE_FORMAT
static int dy_texture_format_lookup[][3] = {

  // Internal,           Layout, Size
    {GL_DEPTH_COMPONENT, GL_RED, GL_UNSIGNED_BYTE}, // DY_TEXTURE_FORMAT_D8
    {GL_DEPTH_STENCIL,   GL_RG,  GL_UNSIGNED_BYTE}, // DY_TEXTURE_FORMAT_DS8

  // Internal, Layout,  Size
    {GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE}, // DY_TEXTURE_FORMAT_RGBA8
    {GL_RGB8,  GL_RGB,  GL_UNSIGNED_BYTE}, // DY_TEXTURE_FORMAT_RGB8
    {GL_RG8,   GL_RG,   GL_UNSIGNED_BYTE}, // DY_TEXTURE_FORMAT_RG8
    {GL_R8,    GL_RED,  GL_UNSIGNED_BYTE}, // DY_TEXTURE_FORMAT_R8
    {GL_RGBA8, GL_BGRA, GL_UNSIGNED_BYTE}, // DY_TEXTURE_FORMAT_BGRA8
    {GL_RGB8,  GL_BGR,  GL_UNSIGNED_BYTE}, // DY_TEXTURE_FORMAT_BGR8

    {GL_RGB32F, GL_RGB, GL_FLOAT}, // DY_TEXTURE_FORMAT_RGB32F
};

void dy_texture_destroy(dy_texture* texture)
{
    dy_texture_data* data = (dy_texture_data*)texture;

    glDeleteTextures(1, &data->id);

    free(data);
}

dy_texture* dy_texture_create(enum DY_TEXTURE_FORMAT format, unsigned char* pixels, unsigned int width, unsigned int height)
{
    // Create the texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Look up the GL format
    int* fmt = &dy_texture_format_lookup[format][0];

    // Put the image in the texture
    glTexImage2D(GL_TEXTURE_2D, 0, fmt[0], width, height, 0, fmt[1], fmt[2], pixels);


    dy_texture_data* data = (dy_texture_data*)malloc(sizeof(dy_texture_data));
    data->id = texture;
    data->width = width;
    data->height = height;
    return (dy_texture*)data;
}

void dy_texture_bind(dy_texture* texture)
{
    dy_texture_data* data = (dy_texture_data*)texture;

    glBindTexture(GL_TEXTURE_2D, data->id);
}

void dy_texture_get_dimensions(dy_texture* texture, unsigned int* width, unsigned int* height)
{
    dy_texture_data* data = (dy_texture_data*)texture;

    if (width)
        *width = data->width;

    if (height)
        *height = data->height;
}




// Framebuffer //


dy_framebuffer* dy_framebuffer_create(dy_texture* colortexture, int hasdepth)
{
    dy_texture_data* txdata = (dy_texture_data*)colortexture;

    // Create the framebuffer
    GLuint framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    
    // Attach the texture to the fb
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, txdata->id, 0);

    GLuint rbo = 0;
    if (hasdepth)
    {
        // Create the depthbuffer
        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, txdata->width, txdata->height);
        
        // Attach it
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
    }

    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

    dy_framebuffer_data* fbdata = (dy_framebuffer_data*)malloc(sizeof(dy_framebuffer_data));
    fbdata->framebuffer = framebuffer;
    fbdata->renderbuffer = rbo;
    fbdata->hasrenderbuffer = hasdepth;
    fbdata->color = (dy_texture_data*)colortexture;
    return (dy_framebuffer*)fbdata;
}


void dy_framebuffer_destroy(dy_framebuffer* framebuffer)
{
    dy_framebuffer_data* data = (dy_framebuffer_data*)framebuffer;
    
    if (data->hasrenderbuffer)
    {
        glDeleteRenderbuffers(1, &data->renderbuffer);
    }
    
    glDeleteFramebuffers(1, &data->framebuffer);
    
    // Leave the color texture as is 
    
    free(data);
}

dy_texture* dy_framebuffer_color(dy_framebuffer* framebuffer)
{
    dy_framebuffer_data* data = (dy_framebuffer_data*)framebuffer;
    return data->color;
}


void dy_framebuffer_bind(dy_framebuffer* framebuffer)
{
    dy_framebuffer_data* data = (dy_framebuffer_data*)framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, data->framebuffer);
}
void dy_framebuffer_unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}