#include "dy_objimport.h"
#include "dy_ustack.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

bool IsEndLine(char c)
{
	return c == '\n' || c == '\r';
}

static char* PullString(char* c, objstring_t* out)
{
	char* start = c;
	for (; *c && !IsEndLine(*c); c++);

	unsigned int len = c - start;
	char* str = (char*)malloc(len + 1u);
	strncpy(str, start, len);
	str[len] = 0;
	
	*out = { str, len };

	return c;
}


char* StringToVec(char* c, vec3* out)
{
	out->x = strtod(c, &c);
	out->y = strtod(c, &c);
	out->z = strtod(c, &c);

	return c;
}


static char* PullTexture(char* c, objtexture_t* out)
{
	return PullString(c, &out->name);
	return c;
}

int LoadMtlString(char* content, mtlfile_t* mtl)
{
	if (!content || !mtl)
		return 0;


	dy_ustack<objmaterial_t> mats;


	objmaterial_t mat;

	for (char* c = content; *c; c++)
	{
		if (isspace(*c))
			continue;
			
		if (strncmp("newmtl ", c, 7) == 0)
		{
			c += 7;

			if (mat.name.str)
			{
				mats.push(mat);

			}
			memset(&mat, 0, sizeof(objmaterial_t));

			c = PullString(c, &mat.name);


		}
		else if (strncmp("Ka ", c, 3) == 0)
		{
			c += 3;
			
			c = StringToVec(c, &mat.ambientColor);
		}
		else if (strncmp("Kd ", c, 3) == 0)
		{
			c += 3;

			c = StringToVec(c, &mat.diffuseColor);
		}
		else if (strncmp("Ks ", c, 3) == 0)
		{
			c += 3;

			c = StringToVec(c, &mat.specularColor);
		}
		else if (strncmp("Kf ", c, 3) == 0)
		{
			c += 3;

			c = StringToVec(c, &mat.transmissionFilterColor);
		}
		else if (strncmp("Kf ", c, 3) == 0)
		{
			c += 3;
		
			mat.specularExponent = strtod(c, &c);
		}
		else if (strncmp("d ", c, 2) == 0)
		{
			c += 2;

			mat.opacity = strtod(c, &c);
		}
		else if (strncmp("Tr ", c, 3) == 0)
		{
			c += 3;

			mat.opacity = 1.0 - strtod(c, &c);
		}
		else if (strncmp("Ni ", c, 3) == 0)
		{
			c += 3;

			mat.opticalDensity = strtod(c, &c);
		}
		else if (strncmp("map_Kd ", c, 7) == 0)
		{
			c += 7;

			c = PullTexture(c, &mat.diffuseTx);
		}
		else if (strncmp("map_Ks ", c, 7) == 0)
		{
			c += 7;

			c = PullTexture(c, &mat.specularTx);
		}
		else if (strncmp("map_Ns ", c, 7) == 0)
		{
			c += 7;

			//c = PullTexture(c, mat.);
		}
		else
		{
			// Unsupported, read to eol
			for (; *c && !IsEndLine(*c); c++);
		}
	}
	return 0;
}

void decodeOBJ(const char* buf, unsigned int size, objmodel_t* out)
{
	if (!buf || !size || !out)
		return;


	dy_ustack<objmesh_t> meshes;
	dy_ustack<objface_t> faces;
	dy_ustack<vec3> norms;
	dy_ustack<vec3> verts;
	dy_ustack<vec2> texcoords;

	objmesh_t curmesh = {{0,0}, 0, 0};

	for (char* c = (char*)buf; *c; c++)
	{
		if (isspace(*c))
			continue;
		/*
		if (strncmp("g ", c, 2) == 0)
		{
			c += 2;

			// New Group
			if(currentgroup.)

			ll_push(&norms, &n);
		}
		else */
		if (strncmp("vn ", c, 3) == 0)
		{
			c += 3;

			// Normal
			vec3 n = { 0,0,0 };
			n.x = strtod(c, &c);
			n.y = strtod(c, &c);
			n.z = strtod(c, &c);

			norms.push(n);
		}
		else if (strncmp("vt ", c, 3) == 0)
		{
			c += 3;

			// Texture Coord
			vec2 t = { 0,0 };
			t.x = strtod(c, &c);
			t.y = 1.0f - strtod(c, &c);

			texcoords.push(t);
		}
		else if (strncmp("v ", c, 2) == 0)
		{
			c += 2;

			// Position
			vec3 v = { 0,0,0 };
			v.x = strtod(c, &c);
			v.y = strtod(c, &c);
			v.z = strtod(c, &c);

			verts.push(v);
		}
		else if (*c == 'f')
		{
			// Face
			// Parse the face til eol
			c += 2;

			dy_ustack<objvert_t> faceverts;

			while (c - buf < size)
			{
				for (; c - buf < size && isspace(*c) && !IsEndLine(*c); c++);
				if (IsEndLine(*c))
					break;

				unsigned int vert = 0, text = 0, norm = 0;

				vert = strtoul(c, &c, 10);
				if (*c == '/')
					text = strtoul(++c, &c, 10);
				if (*c == '/')
					norm = strtoul(++c, &c, 10);

				objvert_t fv;
				fv.vertex = vert - 1;
				fv.uv = text - 1;
				fv.normal = norm - 1;
				
				faceverts.push(fv);
			}
			if (faceverts.count)
			{
				objface_t face;
				face.vertexCount = faceverts.count;
				
				face.vertices = faceverts.packed();

				faces.push(face);
			}
			faceverts.clear();

		}
		else if (strncmp("mtllib ", c, 7) == 0)
		{
			c += 7;
			

			// Copy it into a temp buf so we can null terminate it
			char* start = c;
			for (; *c && !IsEndLine(*c); c++);
			
			unsigned int len = c - start;
			char* matname = (char*)malloc(len + 1u);
			strncpy(matname, start, len);
			matname[len] = 0;
			
			unsigned int fc = faces.count - (unsigned int)curmesh.faces;
			if(fc > 0)
			{
				curmesh.faceCount = fc;
				meshes.push(curmesh);
			}
			curmesh = {{matname, len}, (unsigned short)(faces.count - curmesh.faceCount), reinterpret_cast<objface_t*>(static_cast<uintptr_t>(faces.count))};

		}
		//else if (strncmp("o ", c, 7) == 0)
		else
		{
			// Unsupported, read to eol
			for (; c - buf < size && !IsEndLine(*c); c++);
		}
	}

	unsigned int meshfc = faces.count - (unsigned int)curmesh.faces;
	if(meshfc > 0)
	{
		curmesh.faceCount = meshfc;
		meshes.push(curmesh);
	}

	//out->faceCount = faces.count;
	out->facepool = faces.packed();

	out->normCount = norms.count;
	out->norms = norms.packed();

	out->vertCount = verts.count;
	out->verts = verts.packed();

	out->uvCount = texcoords.count;
	out->uvs = texcoords.packed();

	out->meshCount = meshes.count;
	out->meshes = meshes.packed();

	// Fill in the mesh data with real pointers
	for(int i = 0; i < out->meshCount; i++)
	{
		out->meshes[i].faces = out->facepool + reinterpret_cast<uintptr_t>(out->meshes[i].faces);
	}

}


void freeOBJ(objmodel_t* obj)
{
	for(int i = 0; i < obj->meshCount; i++)
		free(obj->meshes[i].material.str);
	free(obj->facepool);
	free(obj->verts);
	free(obj->norms);
	free(obj->uvs);
}