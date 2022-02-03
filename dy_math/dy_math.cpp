#include "dy_math.h"
#include <math.h>

// Vector 3
vec3 vec3::cross(vec3 const& r) {
	return { y * r.z - z * r.y,z * r.x - x * r.z,x * r.y - y * r.x };
}
float vec3::dot(vec3 const& r) {
	return x * r.x + y * r.y + z * r.z;
}
float vec3::mag() {
	return sqrtf(x * x + y * y + z * z);
}
vec3 vec3::normalized() {
	return *this / mag();
}

vec3 operator+ (vec3 const& l, vec3 const& r) {
	return { l.x + r.x, l.y + r.y, l.z + r.z };
}
vec3 operator- (vec3 const& l, vec3 const& r) {
	return { l.x - r.x, l.y - r.y, l.z - r.z };
}
vec3 operator* (vec3 const& l, float const r) {
	return { l.x * r, l.y * r, l.z * r };
}
vec3 operator/ (vec3 const& l, float const r) {
	return { l.x / r, l.y / r, l.z / r };
}

vec3& operator+= (vec3& l, vec3 const& r) {
    l.x += r.x;
    l.y += r.y;
    l.z += r.z;
    return l;
}
vec3& operator-= (vec3& l, vec3 const& r) {
    l.x -= r.x;
    l.y -= r.y;
    l.z -= r.z;
    return l;
}
vec3& operator*= (vec3& l, float const r) {
    l.x *= r;
    l.y *= r;
    l.z *= r;
    return l;
}
vec3& operator/= (vec3& l, float const r) {
    l.x /= r;
    l.y /= r;
    l.z /= r;
    return l;
}


// Vector 4
float vec4::dot(vec4 const& r) {
	return x * r.x + y * r.y + z * r.z + w * r.w;
}
float vec4::mag() {
	return sqrtf(x * x + y * y + z * z + w * w);
}
vec4 vec4::normalized() {
	return *this / mag();
}

vec4 operator+ (vec4 const& l, vec4 const& r) {
    return { l.x + r.x, l.y + r.y, l.z + r.z, l.w + r.w };
}
vec4 operator- (vec4 const& l, vec4 const& r) {
    return { l.x - r.x, l.y - r.y, l.z - r.z, l.w - r.w };
}
vec4 operator* (vec4 const& l, float const r) {
    return { l.x * r, l.y * r, l.z * r, l.w * r };
}
vec4 operator/ (vec4 const& l, float const r) {
    return { l.x / r, l.y / r, l.z / r, l.w / r };
}

vec4& operator+= (vec4& l, vec4 const& r) {
    l.x += r.x;
    l.y += r.y;
    l.z += r.z;
    l.w += r.w;
    return l;
}
vec4& operator-= (vec4& l, vec4 const& r) {
    l.x -= r.x;
    l.y -= r.y;
    l.z -= r.z;
    l.w -= r.w;
    return l;
}
vec4& operator*= (vec4& l, float const r) {
    l.x *= r;
    l.y *= r;
    l.z *= r;
    l.w *= r;
    return l;
}
vec4& operator/= (vec4& l, float const r) {
    l.x /= r;
    l.y /= r;
    l.z /= r;
    l.w /= r;
    return l;
}

// Matrices

const mat4 mat4::identity() {
    return 
    {{1, 0, 0, 0},
     {0, 1, 0, 0},
     {0, 0, 1, 0},
     {0, 0, 0, 1}};
}

mat4 operator* (mat4 const& l, mat4 const& r) {
    // Unrolled
    mat4 m;
    vec4 row;

    row = { l.a.x, l.b.x, l.c.x, l.d.x };
    m.a.x = row.dot(r.a);
    m.b.x = row.dot(r.b);
    m.c.x = row.dot(r.c);
    m.d.x = row.dot(r.d);

    row = { l.a.y, l.b.y, l.c.y, l.d.y };
    m.a.y = row.dot(r.a);
    m.b.y = row.dot(r.b);
    m.c.y = row.dot(r.c);
    m.d.y = row.dot(r.d);

    row = { l.a.z, l.b.z, l.c.z, l.d.z };
    m.a.z = row.dot(r.a);
    m.b.z = row.dot(r.b);
    m.c.z = row.dot(r.c);
    m.d.z = row.dot(r.d);

    row = { l.a.w, l.b.w, l.c.w, l.d.w };
    m.a.w = row.dot(r.a);
    m.b.w = row.dot(r.b);
    m.c.w = row.dot(r.c);
    m.d.w = row.dot(r.d);

    return m;
}

vec4 operator* (mat4 const& l, vec4 const& r) {
    // Unrolled
    vec4 v;
    v  = l.a * r.x;
    v += l.b * r.y;
    v += l.c * r.z;
    v += l.d * r.w;

    return v;
}



const mat4 mat4::xrotation(float angle)
{
    return 
    {{1.0f,        0.0f,         0.0f, 0.0f},
     {0.0f, cosf(angle), -sinf(angle), 0.0f},
     {0.0f, sinf(angle),  cosf(angle), 0.0f},
     {0.0f,        0.0f,         0.0f, 1.0f}};
}
const mat4 mat4::yrotation(float angle)
{
    return
    {{ cosf(angle), 0.0f, sinf(angle), 0.0f},
     {        0.0f, 1.0f,        0.0f, 0.0f},
     {-sinf(angle), 0.0f, cosf(angle), 0.0f},
     {        0.0f, 0.0f,        0.0f, 1.0f}};
}
const mat4 mat4::zrotation(float angle)
{
    return
    {{cosf(angle), -sinf(angle), 0.0f, 0.0f},
     {sinf(angle),  cosf(angle), 0.0f, 0.0f},
     {       0.0f,         0.0f, 1.0f, 0.0f},
     {       0.0f,         0.0f, 0.0f, 1.0f} };
}



// Camera functions //
void dy_perspective4x4(mat4* out, float fov, float near, float far, float aspect)
{
	float s = 1.0f / tanf((fov / 2) * (DY_PI / 180));

	*out = 
	{{s * aspect, 0,                            0,  0},
	 {         0, s,                            0,  0},
	 {         0, 0,          -far / (far - near), -1},
	 {         0, 0, -(far * near) / (far - near),  0}};
}
void dy_ortho4x4(mat4* out, float l, float r, float t, float b, float near, float far)
{
	*out = 
	{{    2.0f / (r - l),                  0,                            0, 0},
	 {                 0,     2.0f / (t - b),                            0, 0},
	 {                 0,                  0,         -2.0f / (far - near), 0},
	 {-(r + l) / (r - l), -(t + b) / (t - b), -(far + near) / (far - near), 1}};
}
