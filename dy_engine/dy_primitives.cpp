#include "dy_primitives.h"
#include <math.h>

static const float RSQRT2 = 1.0f / sqrtf(2.0f);

// TODO: Should we make this a triangle instead of a rectange?
static dy_vertex s_primitiveLineVB[] = {
	  /* Position      */  /*Colo*/ /*Uv*/ /* Normal              */
    { { 0.5,  0.5,  1.0 }, {1,1,1}, {0,0}, {  RSQRT2,  RSQRT2, 0.0 } },
    { { 0.5, -0.5,  1.0 }, {1,1,1}, {0,0}, {  RSQRT2, -RSQRT2, 0.0 } },
    { {-0.5, -0.5,  1.0 }, {1,1,1}, {0,0}, { -RSQRT2, -RSQRT2, 0.0 } },
    { {-0.5,  0.5,  1.0 }, {1,1,1}, {0,0}, { -RSQRT2,  RSQRT2, 0.0 } },
    { { 0.5,  0.5,  0.0 }, {1,1,1}, {0,0}, {  RSQRT2,  RSQRT2, 0.0 } },
    { {-0.5,  0.5,  0.0 }, {1,1,1}, {0,0}, { -RSQRT2,  RSQRT2, 0.0 } },
    { {-0.5, -0.5,  0.0 }, {1,1,1}, {0,0}, { -RSQRT2, -RSQRT2, 0.0 } },
    { { 0.5, -0.5,  0.0 }, {1,1,1}, {0,0}, {  RSQRT2, -RSQRT2, 0.0 } },
};
static unsigned int s_primitiveLineVBCount = sizeof(s_primitiveLineVB) / sizeof(dy_vertex);

static unsigned short s_primitiveLineIB[] = {
	// Front face
	2, 1, 0,
	0, 3, 2,

	// Back face
	6, 5, 4,
	4, 7, 6,

	// Right face
	2, 3, 5,
	5, 6, 2,

	// Left face
	4, 0, 1,
	1, 7, 4,

	// Top face
	0, 4, 5,
	5, 3, 0,

	// Bottom face
	1, 2, 6,
	6, 7, 1
};
static unsigned int s_primitiveLineIBCount = sizeof(s_primitiveLineIB) / sizeof(unsigned short);

dy_primitive g_primitiveLine = { {&s_primitiveLineVB[0], s_primitiveLineVBCount}, {&s_primitiveLineIB[0], s_primitiveLineIBCount} };




/////////////
//// BOX ////
/////////////

// Precomputed lighting, so that we can differentiate the sides of the box
//   dir = -[-0.6 -1 0.2]; dir = dir/norm(dir);
//   n = [0 1 0; 0 -1 0; 1 0 0; -1 0 0 ; 0 0 1; 0 0 -1];
//   l = (0.5 + 0.5 * (dir * n')) .^ 2 * (0.5) + 0.5;
static vec3 s_DBL[] = {
	{0.9256, 0.9256, 0.9256 }, //  0,  1,  0
	{0.5030, 0.5030, 0.5030 }, //  0, -1,  0
	{0.7839, 0.7839, 0.7839 }, //  1,  0,  0
	{0.5304, 0.5304, 0.5304 }, // -1,  0,  0
	{0.5863, 0.5863, 0.5863 }, //  0,  0,  1
	{0.6708, 0.6708, 0.6708 }, //  0,  0, -1
};


static dy_vertex s_primitiveBoxVB[] = {

	{ { 0, 1, 1 }, s_DBL[0], {0,0}, {  0,  1,  0 } },
	{ { 1, 1, 1 }, s_DBL[0], {0,0}, {  0,  1,  0 } },
	{ { 1, 1, 0 }, s_DBL[0], {0,0}, {  0,  1,  0 } },
	{ { 0, 1, 0 }, s_DBL[0], {0,0}, {  0,  1,  0 } },

	{ { 0, 0, 0 }, s_DBL[1], {0,0}, {  0, -1,  0 } },
	{ { 1, 0, 0 }, s_DBL[1], {0,0}, {  0, -1,  0 } },
	{ { 1, 0, 1 }, s_DBL[1], {0,0}, {  0, -1,  0 } },
	{ { 0, 0, 1 }, s_DBL[1], {0,0}, {  0, -1,  0 } },

	{ { 1, 1, 0 }, s_DBL[2], {0,0}, {  1,  0,  0 } },
	{ { 1, 1, 1 }, s_DBL[2], {0,0}, {  1,  0,  0 } },
	{ { 1, 0, 1 }, s_DBL[2], {0,0}, {  1,  0,  0 } },
	{ { 1, 0, 0 }, s_DBL[2], {0,0}, {  1,  0,  0 } },

	{ { 0, 0, 0 }, s_DBL[3], {0,0}, { -1,  0,  0 } },
	{ { 0, 0, 1 }, s_DBL[3], {0,0}, { -1,  0,  0 } },
	{ { 0, 1, 1 }, s_DBL[3], {0,0}, { -1,  0,  0 } },
	{ { 0, 1, 0 }, s_DBL[3], {0,0}, { -1,  0,  0 } },

	{ { 1, 0, 1 }, s_DBL[4], {0,0}, {  0,  0,  1 } },
	{ { 1, 1, 1 }, s_DBL[4], {0,0}, {  0,  0,  1 } },
	{ { 0, 1, 1 }, s_DBL[4], {0,0}, {  0,  0,  1 } },
	{ { 0, 0, 1 }, s_DBL[4], {0,0}, {  0,  0,  1 } },

	{ { 0, 0, 0 }, s_DBL[5], {0,0}, {  0,  0, -1 } },
	{ { 0, 1, 0 }, s_DBL[5], {0,0}, {  0,  0, -1 } },
	{ { 1, 1, 0 }, s_DBL[5], {0,0}, {  0,  0, -1 } },
	{ { 1, 0, 0 }, s_DBL[5], {0,0}, {  0,  0, -1 } },

};
static unsigned int s_primitiveBoxVBCount = sizeof(s_primitiveBoxVB) / sizeof(dy_vertex);


static unsigned short s_primitiveBoxIB[] = {
	 0,  1,  2,
	 2,  3,  0,

 	 4,  5,  6,
 	 6,  7,  4,
	 
	 8,  9, 10,
	10, 11,  8,

	12, 13, 14,
	14, 15, 12,

	16, 17, 18,
	18, 19, 16,

	20, 21, 22,
	22, 23, 20,
};
static unsigned int s_primitiveBoxIBCount = sizeof(s_primitiveBoxIB) / sizeof(unsigned short);

dy_primitive g_primitiveBox = { {&s_primitiveBoxVB[0], s_primitiveBoxVBCount}, {&s_primitiveBoxIB[0], s_primitiveBoxIBCount} };




////////////////
//// SPHERE ////
////////////////

static dy_vertex s_primitiveSphereVB[] = {
	{ {  0.000000, -1.000000,  0.000000 }, {1,1,1}, {0,0}, {0,1,0} },
	{ {  0.723607, -0.447220,  0.525725 }, {1,1,1}, {0,0}, {0,1,0} },
	{ { -0.276388, -0.447220,  0.850649 }, {1,1,1}, {0,0}, {0,1,0} },
	{ { -0.894426, -0.447216,  0.000000 }, {1,1,1}, {0,0}, {0,1,0} },
	{ { -0.276388, -0.447220, -0.850649 }, {1,1,1}, {0,0}, {0,1,0} },
	{ {  0.723607, -0.447220, -0.525725 }, {1,1,1}, {0,0}, {0,1,0} },
	{ {  0.276388,  0.447220,  0.850649 }, {1,1,1}, {0,0}, {0,1,0} },
	{ { -0.723607,  0.447220,  0.525725 }, {1,1,1}, {0,0}, {0,1,0} },
	{ { -0.723607,  0.447220, -0.525725 }, {1,1,1}, {0,0}, {0,1,0} },
	{ {  0.276388,  0.447220, -0.850649 }, {1,1,1}, {0,0}, {0,1,0} },
	{ {  0.894426,  0.447216,  0.000000 }, {1,1,1}, {0,0}, {0,1,0} },
	{ {  0.000000,  1.000000,  0.000000 }, {1,1,1}, {0,0}, {0,1,0} },
	{ { -0.162456, -0.850654,  0.499995 }, {1,1,1}, {0,0}, {0,1,0} },
	{ {  0.425323, -0.850654,  0.309011 }, {1,1,1}, {0,0}, {0,1,0} },
	{ {  0.262869, -0.525738,  0.809012 }, {1,1,1}, {0,0}, {0,1,0} },
	{ {  0.850648, -0.525736,  0.000000 }, {1,1,1}, {0,0}, {0,1,0} },
	{ {  0.425323, -0.850654, -0.309011 }, {1,1,1}, {0,0}, {0,1,0} },
	{ { -0.525730, -0.850652,  0.000000 }, {1,1,1}, {0,0}, {0,1,0} },
	{ { -0.688189, -0.525736,  0.499997 }, {1,1,1}, {0,0}, {0,1,0} },
	{ { -0.162456, -0.850654, -0.499995 }, {1,1,1}, {0,0}, {0,1,0} },
	{ { -0.688189, -0.525736, -0.499997 }, {1,1,1}, {0,0}, {0,1,0} },
	{ {  0.262869, -0.525738, -0.809012 }, {1,1,1}, {0,0}, {0,1,0} },
	{ {  0.951058,  0.000000,  0.309013 }, {1,1,1}, {0,0}, {0,1,0} },
	{ {  0.951058,  0.000000, -0.309013 }, {1,1,1}, {0,0}, {0,1,0} },
	{ {  0.000000,  0.000000,  1.000000 }, {1,1,1}, {0,0}, {0,1,0} },
	{ {  0.587786,  0.000000,  0.809017 }, {1,1,1}, {0,0}, {0,1,0} },
	{ { -0.951058,  0.000000,  0.309013 }, {1,1,1}, {0,0}, {0,1,0} },
	{ { -0.587786,  0.000000,  0.809017 }, {1,1,1}, {0,0}, {0,1,0} },
	{ { -0.587786,  0.000000, -0.809017 }, {1,1,1}, {0,0}, {0,1,0} },
	{ { -0.951058,  0.000000, -0.309013 }, {1,1,1}, {0,0}, {0,1,0} },
	{ {  0.587786,  0.000000, -0.809017 }, {1,1,1}, {0,0}, {0,1,0} },
	{ {  0.000000,  0.000000, -1.000000 }, {1,1,1}, {0,0}, {0,1,0} },
	{ {  0.688189,  0.525736,  0.499997 }, {1,1,1}, {0,0}, {0,1,0} },
	{ { -0.262869,  0.525738,  0.809012 }, {1,1,1}, {0,0}, {0,1,0} },
	{ { -0.850648,  0.525736,  0.000000 }, {1,1,1}, {0,0}, {0,1,0} },
	{ { -0.262869,  0.525738, -0.809012 }, {1,1,1}, {0,0}, {0,1,0} },
	{ {  0.688189,  0.525736, -0.499997 }, {1,1,1}, {0,0}, {0,1,0} },
	{ {  0.162456,  0.850654,  0.499995 }, {1,1,1}, {0,0}, {0,1,0} },
	{ {  0.525730,  0.850652,  0.000000 }, {1,1,1}, {0,0}, {0,1,0} },
	{ { -0.425323,  0.850654,  0.309011 }, {1,1,1}, {0,0}, {0,1,0} },
	{ { -0.425323,  0.850654, -0.309011 }, {1,1,1}, {0,0}, {0,1,0} },
	{ {  0.162456,  0.850654, -0.499995 }, {1,1,1}, {0,0}, {0,1,0} },
};
static unsigned int s_primitiveSphereVBCount = sizeof(s_primitiveSphereVB) / sizeof(dy_vertex);


static unsigned short s_primitiveSphereIB[] = {
	 0, 13, 12,    1, 13, 15,
	 0, 12, 17,    0, 17, 19,
	 0, 19, 16,    1, 15, 22,
	 2, 14, 24,    3, 18, 26,
	 4, 20, 28,    5, 21, 30,
	 1, 22, 25,    2, 24, 27,
	 3, 26, 29,    4, 28, 31,
	 5, 30, 23,    6, 32, 37,
	 7, 33, 39,    8, 34, 40,
	 9, 35, 41,   10, 36, 38,
	38, 41, 11,   38, 36, 41,
	36,  9, 41,   41, 40, 11,
	41, 35, 40,   35,  8, 40,
	40, 39, 11,   40, 34, 39,
	34,  7, 39,   39, 37, 11,
	39, 33, 37,   33,  6, 37,
	37, 38, 11,   37, 32, 38,
	32, 10, 38,   23, 36, 10,
	23, 30, 36,   30,  9, 36,
	31, 35,  9,   31, 28, 35,
	28,  8, 35,   29, 34,  8,
	29, 26, 34,   26,  7, 34,
	27, 33,  7,   27, 24, 33,
	24,  6, 33,   25, 32,  6,
	25, 22, 32,   22, 10, 32,
	30, 31,  9,   30, 21, 31,
	21,  4, 31,   28, 29,  8,
	28, 20, 29,   20,  3, 29,
	26, 27,  7,   26, 18, 27,
	18,  2, 27,   24, 25,  6,
	24, 14, 25,   14,  1, 25,
	22, 23, 10,   22, 15, 23,
	15,  5, 23,   16, 21,  5,
	16, 19, 21,   19,  4, 21,
	19, 20,  4,   19, 17, 20,
	17,  3, 20,   17, 18,  3,
	17, 12, 18,   12,  2, 18,
	15, 16,  5,   15, 13, 16,
	13,  0, 16,   12, 14,  2,
	12, 13, 14,   13,  1, 14,
};
static unsigned int s_primitiveSphereIBCount = sizeof(s_primitiveSphereIB) / sizeof(unsigned short);


dy_primitive g_primitiveSphere = { {&s_primitiveSphereVB[0], s_primitiveSphereVBCount}, {&s_primitiveSphereIB[0], s_primitiveSphereIBCount} };
