#include "dy_debugtext.h"
#include "dy_texture.h"
#include "dy_shader.h"
#include "dy_vertex.h"
#include "dy_render.h"
#include "dy_asset.h"

#include <string.h>
#include <stdlib.h>

struct dy_fontdata
{
    dy_texture* texture;
    
    unsigned int tilewidth;
    unsigned int tileheight;

    vec4 glyphdims;

    unsigned int columns;
    unsigned int rows;
};

static dy_fontdata s_fd;

dy_fontdata dy_debugtext_importfont(dy_texture* texture, unsigned int tilew, unsigned int tileh, vec4 glyphdims)
{
    unsigned int txw, txh;
    dy_texture_get_dimensions(texture, &txw, &txh);

    dy_fontdata fd;
    fd.texture = texture;
    fd.tilewidth = tilew;
    fd.tileheight = tileh;
    fd.columns = txw / tilew;
    fd.rows = txh / tileh;

    fd.glyphdims = glyphdims;


    return fd;
}

void dy_debugtext_init()
{

    // TEMP
    dy_texture* tx = dy_asset_texture_load("C:/Dev/Calcifer/out/Debug/assets/font.png");

    vec4 gd = vec4{ 3, 0, 13, 16 } / 16.0f;
    s_fd = dy_debugtext_importfont(tx, 16, 16, gd);
}


// TODO: We can precompute, store, and reuse some of this
static void dy_debugtext_buildchar(dy_fontdata* fd, dy_vtxbuf* vb, unsigned char c, float xoff, float yoff)
{
    int x = c % fd->columns;
    int y = c / fd->columns;

    float t = (y + fd->glyphdims.y) / (float) fd->rows;
    float l = (x + fd->glyphdims.x) / (float) fd->columns;
    float b = (y + fd->glyphdims.w) / (float) fd->rows;
    float r = (x + fd->glyphdims.z) / (float) fd->columns;

    dy_vertex v[] =
    {
        {{xoff + 0, yoff + 0,0}, {1,1,1}, {l, b}, {0,0,1}},
        {{xoff + 1, yoff + 0,0}, {1,1,1}, {r, b}, {0,0,1}},
        {{xoff + 1, yoff + 1,0}, {1,1,1}, {r, t}, {0,0,1}},
        {{xoff + 0, yoff + 1,0}, {1,1,1}, {l, t}, {0,0,1}},
    };

    vb->push_many(&v[0], 4);
}


void dy_debugtext(const char* text, unsigned int length, vec3 pos, vec3 campos, float scale)
{
    if (length == 0)
        return;

    dy_vtxbuf vb(length * 4);

    int maxcolumns = 0;

    int row = 0;
    int column = 0;
    int quads = 0;
    for(int i = 0; i < length; i++)
    {
        char c = text[i];
        switch (c)
        {
        case '\r': // Ignore
            break;
        case '\n': // New Line
            row--;
            if (column > maxcolumns) maxcolumns = column;
            column = 0;
            break;
        case ' ': // Space
            column++;
            break;
        case '\t': // Tab
            column+=4;
            break;
        default:
            dy_debugtext_buildchar(&s_fd, &vb, c, column, row);
            column++;
            quads++;
            break;
        }
    }

    if (quads == 0)
        return;

    if (column > maxcolumns) maxcolumns = column;

    dy_idxbuf ib(quads * 6);
    for (int i = 0; i < quads; i++)
        ib.push_quad(i * 4);


    // FIXME: Clean this up and make it work properly!
    vec3 zaxis = (campos - pos).normalized();
    vec3 xaxis = vec3{ 0,1,0 }.cross(zaxis).normalized();
    vec3 yaxis = zaxis.cross(xaxis).normalized();
    mat4 r =
    {{xaxis, 0.0},
     {yaxis, 0.0},
     {zaxis, 0.0},
     {pos,   1.0}};

    mat4 m = mat4::identity();
    m.a.x = scale * (s_fd.glyphdims.z - s_fd.glyphdims.x);
    m.b.y = scale;
    m.d = { -0.5f * maxcolumns * m.a.x, -0.5f * (row+1) * m.b.y, 0, 1 };

    m = r * m;

    dy_shader_set(DY_SHADERPARAM_MODEL, &m);


    dy_texture_bind(s_fd.texture, 0);
    // TODO: Can we do better than this? Please?
    dy_render_draw_dynamic(&vb, &ib);
}
