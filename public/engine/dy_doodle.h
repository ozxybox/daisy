#pragma once

#include "dy_math.h"
#include "dy_halfedge.h"

void dy_doodle_init();
void dy_doodle_render();



// Lives for "decay" seconds
// Using "-1" for "decay" will cause the line to live for only 1 frame
void dy_doodle_line(vec3 start, vec3 end, vec4 color = {0,0,1,1}, float width = 8.0f, float decay = -1.0f);
inline void dy_doodle_line(vec3 start, vec3 end, vec3 color, float width = 8.0f, float decay = -1.0f) { dy_doodle_line(start, end, { color, 1.0 }, width, decay); }

void dy_doodle_aabb(dy_aabb aabb, vec4 color, float bloat = 0.0f, float decay = -1.0f);

void dy_doodle_sphere(vec3 origin, float radius, vec4 color, float decay = -1.0f);

void dy_doodle_rface(dy_rface* face, vec4 color, float dbloat = 1.5f, float decay = -1.0f);
