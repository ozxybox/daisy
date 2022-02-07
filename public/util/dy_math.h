#pragma once

#define DY_EPSILON 0.0001f
#define DY_PI 3.14159265358979323846


// Vectors
struct vec2 {
	float x, y;
};

struct vec3 {
	float x, y, z;

	vec3  cross(vec3 const& r);
	float dot(vec3 const& r);
	float mag();
	vec3 normalized();

	static vec3  cross(vec3 const& l, vec3 const& r);
	static float dot  (vec3 const& l, vec3 const& r);
};

struct vec4 {
	float x, y, z, w;

	float dot(vec4 const& r);
	float mag();
	vec4 normalized();

	static float dot(vec4 const& l, vec4 const& r);
};

vec3 operator+ (vec3 const& l, vec3  const& r);
vec3 operator- (vec3 const& l, vec3  const& r);
vec3 operator* (vec3 const& l, float const  r);
vec3 operator/ (vec3 const& l, float const  r);
vec3 operator* (float const l, vec3  const& r);
vec3 operator/ (float const l, vec3  const& r);

vec3& operator+= (vec3& l, vec3  const& r);
vec3& operator-= (vec3& l, vec3  const& r);
vec3& operator*= (vec3& l, float const  r);
vec3& operator/= (vec3& l, float const  r);

vec4 operator+ (vec4 const& l, vec4  const& r);
vec4 operator- (vec4 const& l, vec4  const& r);
vec4 operator* (vec4 const& l, float const  r);
vec4 operator/ (vec4 const& l, float const  r);
vec4 operator* (float const l, vec4  const& r);
vec4 operator/ (float const l, vec4  const& r);

vec4& operator+= (vec4& l, vec4  const& r);
vec4& operator-= (vec4& l, vec4  const& r);
vec4& operator*= (vec4& l, float const  r);
vec4& operator/= (vec4& l, float const  r);

// Matrices
struct mat3 {
	vec3 a, b, c;

	float det();
	
	// Ax=b
	// Returns true if it can solve for x
	bool solve(vec3 b, vec3* out);

	static const mat3 identity();
};
mat3 operator* (mat3 const& l, mat3 const& r);
vec3 operator* (mat3 const& l, vec3 const& r);

struct mat4 {
	vec4 a, b, c, d;

	static const mat4 identity();
	static const mat4 xrotation(float angle);
	static const mat4 yrotation(float angle);
	static const mat4 zrotation(float angle);
};


mat4 operator* (mat4 const& l, mat4 const& r);
vec4 operator* (mat4 const& l, vec4 const& r);

// Camera functions //
void dy_perspective4x4(mat4* out, float fov, float near, float far, float aspect);
void dy_ortho4x4(mat4* out, float l, float r, float t, float b, float near, float far);


// Misc Utils
float dy_clamp(float value, float min, float max);
vec3 dy_color_hsv(float hue, float saturation, float value);
bool dy_near(float value, float target, float epsilon = DY_EPSILON);