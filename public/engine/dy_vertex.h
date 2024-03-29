#pragma once
#include "dy_math.h"


// Vertex format
typedef struct dy_vertex
{
	vec3 pos;
	vec3 color;
	vec2 uv;
    vec3 norm;
} dy_vertex;




// Basic container for vertices //
class dy_vtxbufview
{
public:
    dy_vertex* buf;
    unsigned int used;
};


// Vertex buffer //
class dy_vtxbuf : public dy_vtxbufview
{
public:

    dy_vtxbuf(unsigned int capacity);
    ~dy_vtxbuf();

    // Resizes the buffer to fit as many elements as needed
    // Don't use this constantly!
    void fit(unsigned int count);

    // Pushes elements to the back of the buffer if there's space
    // We could just pass back a pointer at the current pos, but this should be safer
    void push(dy_vertex* ele);
    void push_many(dy_vertex* arr, unsigned int count);

    // Resets the head back to start. Next push will be at element zero
    void reset();

    unsigned int capacity;

};





// Basic container for indices //
class dy_idxbufview
{
public:
    unsigned short* buf;
    unsigned int used;
};

// Index buffer //
class dy_idxbuf : public dy_idxbufview
{
public:

    dy_idxbuf(unsigned int capacity);
    ~dy_idxbuf();

    // Resets the head back to start. Next push will be at element zero
    void reset();

    // Resizes the buffer to fit as many elements as needed
    // Don't use this constantly!
    void fit(unsigned int count);

    // Pushes elements to the back of the buffer if there's space
    // We could just pass back a pointer at the current pos, but this should be safer
    void push(unsigned short ele);
    void push_many(unsigned short* arr, unsigned int count);
    void push_tri(unsigned short start);
    void push_quad(unsigned short start);
    void push_convexpoly(unsigned short start, unsigned short count);


    unsigned int capacity;


    // Allocates and fills an index buffer based on a vertex buffer and a pattern
    static void fill_tris(dy_idxbuf* out, const dy_vtxbuf* vb);
    static void fill_quads(dy_idxbuf* out, const dy_vtxbuf* vb);
    static void fill_convexpolygon(dy_idxbuf* out, const dy_vtxbuf* vb);
};





// Mesh assembling //

