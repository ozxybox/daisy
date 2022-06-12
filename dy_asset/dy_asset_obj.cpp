#include "dy_asset_obj.h"
#include "dy_decode_obj.h"

#include <stdio.h>
#include <stdlib.h>

dy_model dy_asset_model_load_obj(const char* path)
{
	FILE* f = fopen(path, "rb");
	
	// File does not exist
	if (!f)
	{
		printf("failed to load model from path %s!\n", path);
		return {0,0};
	}

	fseek(f, 0, SEEK_END);
	size_t sz = ftell(f);
	fseek(f, 0, SEEK_SET);
	char* buf = (char*)calloc(sz + 1, 1);
	fread(buf, 1, sz, f);
	fclose(f);

	objmodel_t model;
	decodeOBJ(buf, sz, &model);

	dy_model m { new dy_mesh[model.meshCount], model.meshCount };
	
	for (int meshidx = 0; meshidx < model.meshCount; meshidx++)
	{

		objmesh_t* mesh = &model.meshes[meshidx];

		unsigned int elements = 0;
		for (int i = 0; i < mesh->faceCount; i++)
			elements += mesh->faces[i].vertexCount - 2;
		elements *= 3;


		dy_idxbuf indices(elements);
		unsigned short* idx = indices.buf;
		unsigned int curele = 0;
		for (int i = 0; i < mesh->faceCount; i++)
		{
			// Triangulate
			objface_t* face = mesh->faces + i;
			for (int k = 1; k < face->vertexCount - 1; k++)
			{
				*idx++ = curele;
				*idx++ = curele + k;
				*idx++ = curele + k + 1;
			}
			curele += face->vertexCount;
		}
		indices.used = curele;

		dy_vtxbuf vertices(curele);
		for (int i = 0; i < mesh->faceCount; i++)
		{
			objface_t* face = mesh->faces + i;
			for (int k = 0; k < face->vertexCount; k++)
			{
				objvert_t* ov = &face->vertices[k];
				dy_vertex v;
				v.pos = model.verts[ov->vertex];
				v.norm = model.norms[ov->normal];
				v.uv = model.uvs[ov->uv];
				v.color = { 1,1,1 };
				vertices.push(&v);
			}
		}
		//logInfo("MODEL %d %d", curele, elements);

		dy_vbo vbo = dy_render_create_vbo(&vertices);
		dy_ibo ibo = dy_render_create_ibo(&indices);
		m.meshes[meshidx] = { vbo, ibo, curele };
	}

	freeOBJ(&model);

	return m;
}