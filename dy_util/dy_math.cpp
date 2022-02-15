#include "dy_math.h"
#include <math.h>
#include <assert.h>
#include <float.h>

// Vector 3
vec3 vec3::cross(vec3 const& r) {
	return vec3::cross(*this, r);
}
float vec3::dot(vec3 const& r) {
    return vec3::dot(*this, r);
}
float vec3::mag() {
	return sqrtf(x * x + y * y + z * z);
}
vec3 vec3::normalized() {
	return *this / mag();
}

vec3 vec3::cross(vec3 const& l, vec3 const& r) {
    return { l.y * r.z - l.z * r.y, l.z * r.x - l.x * r.z, l.x * r.y - l.y * r.x };
}
float vec3::dot(vec3 const& l, vec3 const& r) {
    return l.x * r.x + l.y * r.y + l.z * r.z;
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
vec3 operator* (float const l, vec3 const& r) {
    return r * l;
}
vec3 operator/ (float const l, vec3 const& r) {
    return { l / r.x, l / r.y, l / r.z };
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
	return vec4::dot(*this, r);
}
float vec4::mag() {
	return sqrtf(x * x + y * y + z * z + w * w);
}
vec4 vec4::normalized() {
	return *this / mag();
}

float vec4::dot(vec4 const& l, vec4 const& r) {
    return l.x * r.x + l.y * r.y + l.z * r.z + l.w * r.w;
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
vec4 operator* (float const l, vec4 const& r) {
    return r * l;
}
vec4 operator/ (float const l, vec4 const& r) {
    return { l / r.x, l / r.y, l / r.z, l / r.w };
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

const mat3 mat3::identity()
{
    return
    { {1, 0, 0},
     {0, 1, 0},
     {0, 0, 1} };
}

float mat3::det() const
{
    return
    + a.x * (b.y * c.z - b.z * c.y)
    - b.x * (a.y * c.z - a.z * c.y)
    + c.x * (a.y * b.z - a.z * b.y);
}

bool mat3::solve(vec3 b, vec3* out) const
{
    return mat3::solve(*this, b, out);
}

bool mat3::solve(mat3 const& A, vec3 const& b, vec3* out)
{
    // Transpose from column major to row major and augment in b
    float omat[3][4] =
    {{A.a.x, A.b.x, A.c.x, b.x},
     {A.a.y, A.b.y, A.c.y, b.y},
     {A.a.z, A.b.z, A.c.z, b.z}};
    
    // Keep the matrix as pointers so we can swap rows quickly without damaging our augment in omat
    float* mat[3] =
    {&omat[0][0],
     &omat[1][0],
     &omat[2][0]};

    // Get it into row echelon form
    for (int n = 0; n < 2; n++)
    {
        // Find pivots
        float largest = 0;
        float la = 0;
        int pivrow = -1;
        for (int m = n; m < 3; m++)
        {
            float v = mat[m][n];
            float va = fabsf(v);

            if (va > la)
            {
                pivrow = m;
                largest = v;
                la = va;
            }
        }

        // No pivot? No solution!
        if (pivrow == -1)
            return false;

        // Swap pivot to highest
        float* pivot = mat[pivrow];
        mat[pivrow] = mat[n];
        mat[n] = pivot;

        vec4* pivotv = reinterpret_cast<vec4*>(pivot);

        // Apply our pivot row to the rows below 
        for (int m = n + 1; m < 3; m++)
        {
            // Get the multiplier
            float* row = mat[m];
            float v = -row[n] / pivot[n];
            
            vec4* rowv = reinterpret_cast<vec4*>(row);
            *rowv += *pivotv * v;
        }
    }

    // Get it into reduced row echelon form
    for (int n = 2; n; n--)
    {
        for (int m = n - 1; m >= 0; m--)
        {

            float* pivot = mat[n];
            vec4* pivotv = reinterpret_cast<vec4*>(pivot);

            // Get the multiplier
            float* row = mat[m];
            float v = -row[n] / pivot[n];

            // Push that pivot up
            vec4* rowv = reinterpret_cast<vec4*>(row);
            *rowv += *pivotv * v;
        }
    }

    // Clean up our diagonal
    for (int n = 0; n < 3; n++)
    {
        float* rowf = mat[n];
        vec4* row = reinterpret_cast<vec4*>(rowf);

        float v = rowf[n];

        
        // Check for zeros along the diagonal
        if (fabsf(v) <= FLT_EPSILON)
            return false;

        *row /= v;
    }

    // Hoist the augment back off omat
    vec3 v = {mat[0][3], mat[1][3], mat[2][3] };
    *out = v;
    return true;
}

mat3 operator* (mat3 const& l, mat3 const& r)
{
    assert(0);
    return {};
}
vec3 operator* (mat3 const& l, vec3 const& r)
{
    // Unrolled
    vec3 v;
    v  = l.a * r.x;
    v += l.b * r.y;
    v += l.c * r.z;
    

    return v;
}

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


// Misc Utils

float dy_clamp(float value, float min, float max)
{
    if (value > max) return max;
    if (value < min) return min;
    return value;
}

vec3 dy_color_hsv(float hue, float saturation, float value)
{
    const float hueRange = 1.0f;
    const float onesixth = hueRange / 6.0f;
    float oR = dy_clamp((fabsf(hue - hueRange / 2.0f) - onesixth) / onesixth, 0.0f, 1.0f);
    oR = (oR * saturation + (1.0f - saturation)) * value;
    float oG = dy_clamp((onesixth * 2.0f - fabsf(hue - onesixth * 2)) / onesixth, 0.0f, 1.0f);
    oG = (oG * saturation + (1.0f - saturation)) * value;
    float oB = dy_clamp((onesixth * 2.0f - fabsf(hue - onesixth * 4)) / onesixth, 0.0f, 1.0f);
    oB = (oB * saturation + (1.0f - saturation)) * value;
    return { oR, oG, oB };
}

bool dy_near(float value, float target, float epsilon)
{
    return value >= target - epsilon && value < target + epsilon;
}