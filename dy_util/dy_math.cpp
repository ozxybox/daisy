#include "dy_math.h"
#include <math.h>
#include <assert.h>
#include <float.h>

#define DY_MATH_USESIMD 1

#if DY_MATH_USESIMD
#include <xmmintrin.h>
#endif

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
vec3 vec3::abs() {
    return { fabsf(x), fabsf(y), fabsf(z) };
}

float vec3::max() {
    return fmaxf(x, fmaxf(y, z));
}
float vec3::min() {
    return fminf(x, fminf(y, z));
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
vec3 operator+ (vec3 const& l, float const& r) {
	return { l.x + r, l.y + r, l.z + r };
}
vec3 operator- (vec3 const& l, float const& r) {
	return { l.x - r, l.y - r, l.z - r };
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

vec3& operator+= (vec3& l, float const& r) {
    l.x += r;
    l.y += r;
    l.z += r;
    return l;
}
vec3& operator-= (vec3& l, float const& r) {
    l.x -= r;
    l.y -= r;
    l.z -= r;
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


vec3 vec3::min(vec3 const& l, vec3 const& r) {
    return { fminf(l.x, r.x), fminf(l.y, r.y), fminf(l.z, r.z) };
}

vec3 vec3::max(vec3 const& l, vec3 const& r) {
    return { fmaxf(l.x, r.x), fmaxf(l.y, r.y), fmaxf(l.z, r.z) };
}

vec3 vec3::ew_div(vec3 const& l, vec3 const& r) {
    return { l.x / r.x, l.y / r.y, l.z / r.z };
}


// Vector 4
float vec4::dot(vec4 const& r) {
	return vec4::dot(*this, r);
}
float vec4::mag() {
    return vec4::mag(*this);
}
vec4 vec4::normalized() {
    return vec4::normalize(*this);
}

vec4 operator* (float const l, vec4 const& r) {
    return r * l;
}

#if !DY_MATH_USESIMD
float vec4::dot(vec4 const& l, vec4 const& r) {
    return l.x * r.x + l.y * r.y + l.z * r.z + l.w * r.w;
}
float vec4::mag(vec4 const& v) {
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
}
vec4 vec4::normalize(vec4 const& v) {
    return v / v.mag();
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
#else
float vec4::dot(vec4 const& l, vec4 const& r) {
    __m128 s, d = _mm_mul_ps(*(__m128*)&l, *(__m128*)&r);
    s = _mm_shuffle_ps(d, d, _MM_SHUFFLE(2, 3, 0, 1));
    d = _mm_add_ps(d, s);
    s = _mm_movehl_ps(s, d);
    d = _mm_add_ss(d, s);
    return *(float*)&d;
}
float vec4::mag(vec4 const& v) {
    __m128 s, d;
    d = *(__m128*)&v;
    d = _mm_mul_ps(d, d); 
    s = _mm_shuffle_ps(d, d, _MM_SHUFFLE(2, 3, 0, 1));
    d = _mm_add_ps(d, s);
    s = _mm_movehl_ps(s, d);
    d = _mm_add_ss(d, s);
    d = _mm_rsqrt_ss(d);
    return *(float*)&d;
}
vec4 vec4::normalize(vec4 const& v) {

    __m128 s, d;
    d = *(__m128*)&v;
    d = _mm_mul_ps(d, d);
    s = _mm_shuffle_ps(d, d, _MM_SHUFFLE(2, 3, 0, 1));
    d = _mm_add_ps(d, s);
    s = _mm_movehl_ps(s, d);
    d = _mm_add_ss(d, s);
    d = _mm_rsqrt_ss(d);
    d = _mm_shuffle_ps(d, d, 0);

    *(__m128*)&v = _mm_mul_ps(*(__m128*)&v, d);
    return v;
}


vec4 operator+ (vec4 const& l, vec4 const& r) {
    __m128 v = _mm_add_ps(*(__m128*)&l, *(__m128*)&r);
    return *(vec4*)&v;
}
vec4 operator- (vec4 const& l, vec4 const& r) {
    __m128 v = _mm_sub_ps(*(__m128*)&l, *(__m128*)&r);
    return *(vec4*)&v;
}
vec4 operator* (vec4 const& l, float const r) {
    __m128 v = _mm_mul_ps(*(__m128*)&l, _mm_set_ps1(r));
    return *(vec4*)&v;
}
vec4 operator/ (vec4 const& l, float const r) {
    __m128 v = _mm_div_ps(*(__m128*)&l, _mm_set_ps1(r));
    return *(vec4*)&v;
}
vec4 operator/ (float const l, vec4 const& r) {
    __m128 v = _mm_div_ps(_mm_set_ps1(l), *(__m128*)&r);
    return *(vec4*)&v;
}

vec4& operator+= (vec4& l, vec4 const& r) {
    *(__m128*)&l = _mm_add_ps(*(__m128*)&l, *(__m128*)&r);
    return l;
}
vec4& operator-= (vec4& l, vec4 const& r) {
    *(__m128*)&l = _mm_sub_ps(*(__m128*)&l, *(__m128*)&r);
    return l;
}
vec4& operator*= (vec4& l, float const r) {
    *(__m128*)&l = _mm_mul_ps(*(__m128*)&l, _mm_set_ps1(r));
    return l;
}
vec4& operator/= (vec4& l, float const r) {
    *(__m128*)&l = _mm_div_ps(*(__m128*)&l, _mm_set_ps1(r));
    return l;
}
#endif


// Matrix 3x3

const mat3 mat3::identity() {
    return
    {{1, 0, 0},
     {0, 1, 0},
     {0, 0, 1}};
}


const mat3 mat3::xrotation(float angle) {
    return 
    {{1.0f,        0.0f,         0.0f},
     {0.0f, cosf(angle), -sinf(angle)},
     {0.0f, sinf(angle),  cosf(angle)}};
}
const mat3 mat3::yrotation(float angle) {
    return
    {{ cosf(angle), 0.0f, sinf(angle)},
     {        0.0f, 1.0f,        0.0f},
     {-sinf(angle), 0.0f, cosf(angle)}};
}
const mat3 mat3::zrotation(float angle) {
    return
    {{cosf(angle), -sinf(angle), 0.0f},
     {sinf(angle),  cosf(angle), 0.0f},
     {       0.0f,         0.0f, 1.0f}};
}

float mat3::det() const
{
    return
    + a.x * (b.y * c.z - b.z * c.y)
    - b.x * (a.y * c.z - a.z * c.y)
    + c.x * (a.y * b.z - a.z * b.y);
}

bool mat3::solve(vec3 const& b, vec3* out) const
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
    // Unrolled
    mat3 m;
    vec3 row;

    row = { l.a.x, l.b.x, l.c.x };
    m.a.x = row.dot(r.a);
    m.b.x = row.dot(r.b);
    m.c.x = row.dot(r.c);

    row = { l.a.y, l.b.y, l.c.y };
    m.a.y = row.dot(r.a);
    m.b.y = row.dot(r.b);
    m.c.y = row.dot(r.c);

    row = { l.a.z, l.b.z, l.c.z };
    m.a.z = row.dot(r.a);
    m.b.z = row.dot(r.b);
    m.c.z = row.dot(r.c);

    return m;
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


// Matrix 4x4
const mat4 mat4::identity() {
    return 
    {{1, 0, 0, 0},
     {0, 1, 0, 0},
     {0, 0, 1, 0},
     {0, 0, 0, 1}};
}

#if !DY_MATH_USESIMD
const mat4 mat4::mul(mat4 const& l, mat4 const& r) {
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

const vec4 mat4::mul(mat4 const& l, vec4 const& r) {
    // Unrolled
    vec4 v;
    v  = l.a * r.x;
    v += l.b * r.y;
    v += l.c * r.z;
    v += l.d * r.w;

    return v;
}
#else
const mat4 mat4::mul(mat4 const& l, mat4 const& r) {
    // Unrolled
    const __m128* pl = reinterpret_cast<const __m128*>(&l);
    __m128 m[4];
    
    __m128 x,y,z,w, v;

    v = _mm_load_ps(&r.a.x);
    x = _mm_shuffle_ps(v, v, _MM_SHUFFLE(0, 0, 0, 0));
    y = _mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 1, 1, 1));
    z = _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 2, 2, 2));
    w = _mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 3, 3, 3));

    m[0] = _mm_add_ps(
            _mm_add_ps(
             _mm_mul_ps(pl[0], x),
             _mm_mul_ps(pl[1], y)
            ),
            _mm_add_ps(
             _mm_mul_ps(pl[2], z),
             _mm_mul_ps(pl[3], w)
            )
           );

    v = _mm_load_ps(&r.b.x);
    x = _mm_shuffle_ps(v, v, _MM_SHUFFLE(0, 0, 0, 0));
    y = _mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 1, 1, 1));
    z = _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 2, 2, 2));
    w = _mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 3, 3, 3));

    m[1] = 
        _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(pl[0], x),
                _mm_mul_ps(pl[1], y)
            ),
            _mm_add_ps(
                _mm_mul_ps(pl[2], z),
                _mm_mul_ps(pl[3], w)
            ));
    
    v = _mm_load_ps(&r.c.x);
    x = _mm_shuffle_ps(v, v, _MM_SHUFFLE(0, 0, 0, 0));
    y = _mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 1, 1, 1));
    z = _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 2, 2, 2));
    w = _mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 3, 3, 3));

    m[2] =
        _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(pl[0], x),
                _mm_mul_ps(pl[1], y)
            ),
            _mm_add_ps(
                _mm_mul_ps(pl[2], z),
                _mm_mul_ps(pl[3], w)
            ));
    
    v = _mm_load_ps(&r.d.x);
    x = _mm_shuffle_ps(v, v, _MM_SHUFFLE(0, 0, 0, 0));
    y = _mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 1, 1, 1));
    z = _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 2, 2, 2));
    w = _mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 3, 3, 3));

    m[3] =
        _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(pl[0], x),
                _mm_mul_ps(pl[1], y)
            ),
            _mm_add_ps(
                _mm_mul_ps(pl[2], z),
                _mm_mul_ps(pl[3], w)
            ));

    return *reinterpret_cast<mat4*>(m);
}

const vec4 mat4::mul(mat4 const& l, vec4 const& r) {
    // Unrolled
    __m128 v = *reinterpret_cast<const __m128*>(&r);

    v =
        _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(*(__m128*)&l.a, _mm_shuffle_ps(v, v, _MM_SHUFFLE(0, 0, 0, 0))),
                _mm_mul_ps(*(__m128*)&l.b, _mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 1, 1, 1)))),
            _mm_add_ps(
                _mm_mul_ps(*(__m128*)&l.c, _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 2, 2, 2))),
                _mm_mul_ps(*(__m128*)&l.d, _mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 3, 3, 3))))
        );

    return *reinterpret_cast<vec4*>(&v);
}
#endif



const mat4 mat4::xrotation(float angle) {
    return 
    {{1.0f,        0.0f,         0.0f, 0.0f},
     {0.0f, cosf(angle), -sinf(angle), 0.0f},
     {0.0f, sinf(angle),  cosf(angle), 0.0f},
     {0.0f,        0.0f,         0.0f, 1.0f}};
}
const mat4 mat4::yrotation(float angle) {
    return
    {{ cosf(angle), 0.0f, sinf(angle), 0.0f},
     {        0.0f, 1.0f,        0.0f, 0.0f},
     {-sinf(angle), 0.0f, cosf(angle), 0.0f},
     {        0.0f, 0.0f,        0.0f, 1.0f}};
}
const mat4 mat4::zrotation(float angle) {
    return
    {{cosf(angle), -sinf(angle), 0.0f, 0.0f},
     {sinf(angle),  cosf(angle), 0.0f, 0.0f},
     {       0.0f,         0.0f, 1.0f, 0.0f},
     {       0.0f,         0.0f, 0.0f, 1.0f}};
}

const mat4 mat4::scale(vec4 const& v) {
    return
    {{v.x, 0.0, 0.0, 0.0},
     {0.0, v.y, 0.0, 0.0},
     {0.0, 0.0, v.z, 0.0},
     {0.0, 0.0, 0.0, v.w}};
}



vec4 operator* (mat4 const& l, vec4 const& r) { return mat4::mul(l, r); }
mat4 operator* (mat4 const& l, mat4 const& r) { return mat4::mul(l, r); }



// Camera functions //
void dy_perspective4x4(mat4* out, float fov, float near, float far, float aspect) {
	float s = 1.0f / tanf((fov / 2) * (DY_PI / 180));

	*out = 
	{{s * aspect, 0,                            0,  0},
	 {         0, s,                            0,  0},
	 {         0, 0,          -far / (far - near), -1},
	 {         0, 0, -(far * near) / (far - near),  0}};
}
void dy_ortho4x4(mat4* out, float l, float r, float t, float b, float near, float far) {
	*out = 
	{{    2.0f / (r - l),                  0,                            0, 0},
	 {                 0,     2.0f / (t - b),                            0, 0},
	 {                 0,                  0,         -2.0f / (far - near), 0},
	 {-(r + l) / (r - l), -(t + b) / (t - b), -(far + near) / (far - near), 1}};
}
vec4 dy_unproject4x4(mat4* proj, vec4 v) {
    v.x /= proj->a.x;
    v.y /= proj->b.y;
    v.z /= proj->c.z;
    v.x += v.w * proj->d.x;
    v.y += v.w * proj->d.y;
    v.z += v.w * proj->d.z;
    return v;
}

// Misc Utils

float dy_clamp(float value, float min, float max) {
    if (value > max) return max;
    if (value < min) return min;
    return value;
}

vec3 dy_color_hsv(float hue, float saturation, float value) {
    const float hueRange = 1.0f;
    const float onesixth = hueRange / 6.0f;
    hue = fmodf(hue, hueRange);
    float oR = dy_clamp((fabsf(hue - hueRange / 2.0f) - onesixth) / onesixth, 0.0f, 1.0f);
    oR = (oR * saturation + (1.0f - saturation)) * value;
    float oG = dy_clamp((onesixth * 2.0f - fabsf(hue - onesixth * 2)) / onesixth, 0.0f, 1.0f);
    oG = (oG * saturation + (1.0f - saturation)) * value;
    float oB = dy_clamp((onesixth * 2.0f - fabsf(hue - onesixth * 4)) / onesixth, 0.0f, 1.0f);
    oB = (oB * saturation + (1.0f - saturation)) * value;
    return { oR, oG, oB };
}

bool dy_near(float value, float target, float epsilon) {
    return value >= target - epsilon && value < target + epsilon;
}


vec3 dy_vec_forward(vec3 rotation) {
    // TODO: Condense this into one matrix!
    return mat3::yrotation(rotation.yaw) * mat3::xrotation(rotation.pitch) * vec3{0,0,-1};
}



dy_transform::dy_transform(vec3 const& _origin, vec3 const& _rotation, vec3 const& _scale)
    : origin(_origin), rotation(_rotation), scale(_scale) { }



mat4 dy_transform::matrix() {
    mat4 model;
    
    // Scale it up
    model = mat4::scale({ scale, 1.0 });

    // Rotate it 
    model = mat4::yrotation(rotation.yaw) * mat4::xrotation(rotation.pitch) * mat4::zrotation(rotation.roll) * model;

    // Translate it
    model.d.x = origin.x;
    model.d.y = origin.y;
    model.d.z = origin.z;

    return model;
}

mat4 dy_camera::matrix() {
    mat4 view;

    // Displacement
    view = 
    {{        1,         0,         0, 0},
     {        0,         1,         0, 0},
     {        0,         0,         1, 0},
     {-origin.x, -origin.y, -origin.z, 1}};
    
    // Rotation
    view = mat4::zrotation(-rotation.roll) * mat4::xrotation(-rotation.pitch) * mat4::yrotation(-rotation.yaw) * view;

    return view;
}


#if 0

int sphereTriangleTest(float radius, vec3 origin, tri_t tri, vec3 norm, vec3* clip)
{
    // Did we hit and are we close enough to clip?
    float t = 9e9;
    vec3 p;
    int hit = rayPlaneTest((ray_t){origin, scalev3(norm, -1.0f)}, norm, tri.a, &p, &t);
    if (!hit || t > radius)
        return 0;

    // Edges
    vec3 e[][2] = {
        {tri.a, subv3(tri.b,tri.a)},
        {tri.b, subv3(tri.c,tri.b)},
        {tri.c, subv3(tri.a,tri.c)},
    };

    // Is our point in our tri?
    int intri = 1;
    for(int i = 0; i < 3 && intri; i++)
    	intri &= dotv3(crossv3(e[i][1], norm),subv3(p,e[i][0])) <= 0;
    if(intri)
    {
        // Inside the tri
        *clip = scalev3(norm, radius - t); 
        //logInfo("AA %f - %f %f %f", 1.0f - t / radius, clip->x, clip->y, clip->z);
        return 1;
    }

    // Did we intersect with an edge?
    float r2 = radius*radius;
    for(int i = 0; i < 3; i++)
    {
        float m, b, g, d0, d1, d;
        vec3 u, f, c;

        // Quadratic equation to find the intersection of the edge with the sphere
        m = magv3(e[i][1]);
        u = scalev3(e[i][1],1.0f/m);
        f = subv3(origin,e[i][0]);
        b = dotv3(u,f); 
        g = b*b-dotv3(f,f)+r2;

        if(g <= 0)
            continue;
        g = sqrt(g);
        
        // 2 POIs
        d0 = b-g;
        d1 = b+g;

        // Are we on the edge at all?
        // d0 < b < d1
        if(d1 < 0 || d0 > m)
            continue; 

        // Put it all together
        d = b + (d0>=0?g:0) - (d1 <= m?g:0);
        
        // Clip
        c = subv3(origin, addv3(e[i][0], scalev3(u, d)));
        float q = 1.0f - magv3(c) / radius;

        // Skip exact perfect intersections
        if(q <= EPSILON)
            continue;

        // Return the clip
        c = scalev3(c, q);
        *clip = c;
        //logInfo("BB %f - %f %f %f", q, clip->x, clip->y, clip->z);
        return 1;
    }

    // Nothing
    return 0;
}

#endif

