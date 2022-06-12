#pragma once
#include "util/dy_math.h"



typedef struct 
{
	char* str;
	unsigned int len;
} objstring_t;

// Material files

typedef struct
{
	objstring_t name;

	//vec3 origin;
	//vec3 scale;
	//vec3 turbulence;
	//bool blendu;
	//bool blendv;
	//bool clamp;
	//float boost;
	//float modifyMap;
	//vec2 resolution;
	//float bumpMultiplier;
	//char imfchan;
} objtexture_t;

typedef struct
{
	objstring_t name;
	
	vec3 ambientColor;
	vec3 diffuseColor;
	vec3 specularColor;
	float specularExponent;
	float opacity;
	vec3 transmissionFilterColor;
	float opticalDensity;
	int illuminationMode;

	objtexture_t ambientTx;
	objtexture_t diffuseTx;
	objtexture_t specularTx;
	objtexture_t alphaTx;
	objtexture_t bumpTx;
	objtexture_t displacementTx;
	objtexture_t decalTx;
} objmaterial_t;

typedef struct
{
	unsigned int materialCount;
	objmaterial_t* materials;
}  mtlfile_t;




// Model Files

typedef struct 
{
	unsigned short vertex;
	unsigned short normal;
	unsigned short uv;
} objvert_t;

typedef struct 
{
	int vertexCount;
	objvert_t* vertices;
} objface_t;


typedef struct 
{
	objstring_t material;

	unsigned short faceCount;
	objface_t* faces;
} objmesh_t;


typedef struct
{
	unsigned short meshCount;
	objmesh_t* meshes;

	unsigned short vertCount;
	vec3* verts;

	unsigned short normCount;
	vec3* norms;

	unsigned short uvCount;
	vec2* uvs;

	// Don't use this! The meshes just refer back to it
	objface_t* facepool;
}  objmodel_t;

/*
typedef struct
{
	objmodel_t* models;
	unsigned int modelCount;

	mtlfile_t* materials;
	unsigned int materialCount;

	// Bulk storage
	char* strings;
}  objresult_t;
*/


void decodeOBJ(const char* buf, unsigned int size, objmodel_t* out);

// Does not free the pointer passed in! Only the content!
void freeOBJ(objmodel_t* obj);
