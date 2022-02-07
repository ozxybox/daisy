#include "dy_vertex.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Vertex buffer //

dy_vtxbuf::dy_vtxbuf(unsigned int capacity)
{
    this->capacity = capacity;
    used = 0;
    buf = (dy_vertex*)malloc(sizeof(dy_vertex) * capacity);
}

dy_vtxbuf::~dy_vtxbuf()
{
    if (buf)
        free(buf);
}

void dy_vtxbuf::fit(unsigned int count)
{
    if (capacity >= count)
        return;

    // Reallocate and copy in the old contents. Slow!
    dy_vertex* n = (dy_vertex*)malloc(count * sizeof(dy_vertex));
    memcpy(n, buf, capacity * sizeof(dy_vertex));
    free(buf);

    buf = n;
    capacity = count;
}

void dy_vtxbuf::push(dy_vertex* ele)
{
    // Are we full?
    if (used >= capacity)
    {
        assert(0);
        return;
    }

    buf[used] = *ele;
    used++;
}

void dy_vtxbuf::push_many(dy_vertex* arr, unsigned int count)
{
    // Are we full?
    if (used >= capacity)
    {
        assert(!count);
        return;
    }

    // Cap our copy to prevent overfilling
    if (used + count > capacity)
        count = capacity - used;

    memcpy(buf + used, arr, count * sizeof(dy_vertex));
    used += count;
}

void dy_vtxbuf::reset()
{
    used = 0;
}

/*
void dy_vtxbuf::view(dy_vertex* arr, unsigned int size, dy_vtxbuf* out)
{
    unsigned int count = size / sizeof(dy_vertex);
    out->buf = arr;
    out->capacity = count;
    out->used = count;
}
*/



// Index buffer //

dy_idxbuf::dy_idxbuf(unsigned int capacity)
{
    this->capacity = capacity;
    used = 0;
    buf = (unsigned short*)malloc(sizeof(unsigned short) * capacity);
}

dy_idxbuf::~dy_idxbuf()
{
    if (buf)
        free(buf);

    capacity = 0;
    used = 0;
    buf = 0;
}

void dy_idxbuf::fit(unsigned int count)
{
    if (capacity >= count)
        return;

    // Reallocate and copy in the old contents. Slow!
    unsigned short* n = (unsigned short*)malloc(count * sizeof(unsigned short));
    memcpy(n, buf, capacity * sizeof(unsigned short));
    free(buf);

    buf = n;
    capacity = count;
}

void dy_idxbuf::push(unsigned short ele)
{
    // Are we full?
    if (used >= capacity)
    {
        assert(0);
        return;
    }

    buf[used] = ele;
    used++;
}

void dy_idxbuf::push_many(unsigned short* arr, unsigned int count)
{
    // Are we full?
    if (used >= capacity)
    {
        assert(!count);
        return;
    }

    // Cap our copy to prevent overfilling
    if (used + count > capacity)
        count = capacity - used;

    memcpy(buf + used, arr, count * sizeof(unsigned short));
    used += count;
}

void dy_idxbuf::reset()
{
    used = 0;
}

/*
void dy_idxbuf::view(unsigned short* arr, unsigned int size, dy_idxbuf* out)
{
    unsigned int count = size / sizeof(unsigned short);
    out->buf = arr;
    out->capacity = count;
    out->used = count;
}

*/


// Mesh assembling //

void dy_idxbuf::push_tri(unsigned short start)
{
    unsigned short a[] = {
        start + 0,
        start + 1,
        start + 2
    };
    push_many(&a[0], 3);
}
void dy_idxbuf::push_quad(unsigned short start)
{
    unsigned short a[] = {
        start + 0,
        start + 1,
        start + 2,

        start + 2,
        start + 3,
        start + 0
    };
    push_many(&a[0], 6);
}

void dy_idxbuf::push_convexpoly(unsigned short start, unsigned short count)
{
    int idxCount = (count - 2) * 3;

    unsigned short* bf = buf + used;
    used += idxCount;

    for (unsigned int i = 1; i < count - 1; i++)
    {
        bf[0] = start;
        bf[1] = start + i;
        bf[2] = start + i + 1;
        bf += 3;
    }
}
