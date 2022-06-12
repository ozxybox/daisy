#pragma once

#define DY_EPSILON 0.0001f
#define DY_PI 3.14159265358979323846


// TODO: Can we do this via something cleaner than macros?
#define DY_VEC2_SWIZZLE               \
inline vec2 xx() { return {x,x}; }    \
inline vec2 xy() { return {x,y}; }    \
inline vec2 yy() { return {y,y}; }    \
inline vec2 yx() { return {y,x}; }    \

// TODO: Can we do this via something cleaner than macros?
#define DY_VEC3_SWIZZLE               \
/* Add Z to our vec2 swizzles*/       \
DY_VEC2_SWIZZLE                       \
inline vec2 xz() { return {x,z}; }    \
inline vec2 yz() { return {y,z}; }    \
inline vec2 zz() { return {z,z}; }    \
inline vec2 zx() { return {z,x}; }    \
inline vec2 zy() { return {z,y}; }    \
                                      \
inline vec3 xxx() { return {x,x,x}; } \
inline vec3 xxy() { return {x,x,y}; } \
inline vec3 xxz() { return {x,x,z}; } \
                                      \
inline vec3 xyx() { return {x,y,x}; } \
inline vec3 xyy() { return {x,y,y}; } \
inline vec3 xyz() { return {x,y,z}; } \
                                      \
inline vec3 xzx() { return {x,z,x}; } \
inline vec3 xzy() { return {x,z,y}; } \
inline vec3 xzz() { return {x,z,z}; } \
                                      \
inline vec3 yxx() { return {y,x,x}; } \
inline vec3 yxy() { return {y,x,y}; } \
inline vec3 yxz() { return {y,x,z}; } \
                                      \
inline vec3 yyx() { return {y,y,x}; } \
inline vec3 yyy() { return {y,y,y}; } \
inline vec3 yyz() { return {y,y,z}; } \
                                      \
inline vec3 yzx() { return {y,z,x}; } \
inline vec3 yzy() { return {y,z,y}; } \
inline vec3 yzz() { return {y,z,z}; } \
                                      \
inline vec3 zxx() { return {z,x,x}; } \
inline vec3 zxy() { return {z,x,y}; } \
inline vec3 zxz() { return {z,x,z}; } \
                                      \
inline vec3 zyx() { return {z,y,x}; } \
inline vec3 zyy() { return {z,y,y}; } \
inline vec3 zyz() { return {z,y,z}; } \
                                      \
inline vec3 zzx() { return {z,z,x}; } \
inline vec3 zzy() { return {z,z,y}; } \
inline vec3 zzz() { return {z,z,z}; } 

// Vec4 Swizzle is NOT to be made!
// That would be 256 different functions!
// Maybe something like SIMD's shuffle would be nice?
// Or even, since it's 4 floats, outright using a SIMD shuffle


// Vector 2
union vec2 {
	
	// Position
	struct {
		float x, y;
	};

	// Texture Coordinates
	struct {
		float u, v;
	};



	DY_VEC2_SWIZZLE
};

// Vector 3
union vec3 {


	// Position
	struct {
		float x, y, z;
	};

	// Rotation
	struct {
		float pitch, yaw, roll;
	};

	// Color (RGB)
	struct {
		float r, g, b;
	};

	// Color (HSV)
	struct {
		float hue, saturation, value;
	};

	DY_VEC3_SWIZZLE

	// Static operator functions
	static vec3  cross(vec3 const& l, vec3 const& r);
	static float dot  (vec3 const& l, vec3 const& r);

	static vec3 min(vec3 const& l, vec3 const& r);
	static vec3 max(vec3 const& l, vec3 const& r);
	
	// Elementwise operators
	static vec3 ew_div(vec3 const& l, vec3 const& r);


	vec3  cross(vec3 const& r);
	float dot(vec3 const& r);
	float mag();
	vec3  normalized();
	
	// Absolute value of all components
	vec3  abs();

	// Greatest component
	float max();

	// Least component
	float min();

};

// Vector 4
union vec4 {

	// Position
	struct {
		float x, y, z, w;
	};

	// Color (RGBA)
	struct {
		float r, g, b, a;
	};

	vec4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {};
	vec4(float _x, float _y, float _z          ) : x(_x), y(_y), z(_z), w( 0) {};
	vec4(float _x, float _y                    ) : x(_x), y(_y), z( 0), w( 0) {};
	vec4(float _x                              ) : x(_x), y( 0), z( 0), w( 0) {};
	vec4(                                      ) : x( 0), y( 0), z( 0), w( 0) {};

	vec4(vec3 const& v, float _w) : x(v.x), y(v.y), z(v.z), w(_w) {};

	DY_VEC3_SWIZZLE

	static float dot(vec4 const& l, vec4 const& r);
	static float mag(vec4 const& v);
	static vec4 normalize(vec4 const& v);
	
	float dot(vec4 const& r);
	float mag();
	vec4 normalized();

};

// Matrix 3x3
struct mat3 {
	vec3 a, b, c;

	float det() const;
	
	// Ax=b
	// Returns true if it can solve for x
	bool solve(vec3 const& b, vec3* out) const;

	static bool solve(mat3 const& A, vec3 const& b, vec3* out);
	static const mat3 identity();

	static const mat3 xrotation(float angle);
	static const mat3 yrotation(float angle);
	static const mat3 zrotation(float angle);
};


// Matrix 4x4
struct mat4 {
	vec4 a, b, c, d;

	static const mat4 identity();

	static const mat4 xrotation(float angle);
	static const mat4 yrotation(float angle);
	static const mat4 zrotation(float angle);
	static const mat4 scale(vec4 const& v);

	static const mat4 mul(mat4 const& l, mat4 const& r);
	static const vec4 mul(mat4 const& l, vec4 const& r);
};


// Vector 3 Operators
vec3 operator+ (vec3 const& l, vec3  const& r);
vec3 operator- (vec3 const& l, vec3  const& r);
vec3 operator+ (vec3 const& l, float const& r);
vec3 operator- (vec3 const& l, float const& r);
vec3 operator* (vec3 const& l, float const  r);
vec3 operator/ (vec3 const& l, float const  r);
vec3 operator* (float const l, vec3  const& r);
vec3 operator/ (float const l, vec3  const& r);

vec3& operator+= (vec3& l, vec3  const& r);
vec3& operator-= (vec3& l, vec3  const& r);

vec3& operator+= (vec3& l, float const& r);
vec3& operator-= (vec3& l, float const& r);

vec3& operator*= (vec3& l, float const  r);
vec3& operator/= (vec3& l, float const  r);

// Vector 4 Operators
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

// Matrix 3 Operators
mat3 operator* (mat3 const& l, mat3 const& r);
vec3 operator* (mat3 const& l, vec3 const& r);

// Matrix 4 Operators
mat4 operator* (mat4 const& l, mat4 const& r);
vec4 operator* (mat4 const& l, vec4 const& r);


// Camera functions //
void dy_perspective4x4(mat4* out, float fov, float near, float far, float aspect);
void dy_ortho4x4(mat4* out, float l, float r, float t, float b, float near, float far);
vec4 dy_unproject4x4(mat4* proj, vec4 v);


// Misc Utils
float dy_clamp(float value, float min, float max);
vec3 dy_color_hsv(float hue, float saturation, float value);
bool dy_near(float value, float target, float epsilon = DY_EPSILON);

// Rotation
vec3 dy_vec_forward(vec3 rotation);


// Model Matrix Transform
struct dy_transform {
	dy_transform(vec3 const& _origin = {0,0,0}, vec3 const& _rotation = {0,0,0}, vec3 const& _scale = {1,1,1});

	vec3 origin;
	vec3 rotation;
	vec3 scale;

	mat4 matrix();
};


struct dy_camera {
	vec3 origin;
	vec3 rotation;

	mat4 matrix();
};