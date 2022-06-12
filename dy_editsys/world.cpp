#include "world.h"
#include "dy_ustack.h"
#include <float.h>


// ------------------- //
//     World Tests     //
// ------------------- //


dy_ray::dy_ray(vec3 _start, vec3 _end) :
	start(_start), t(FLT_MAX), dir(_end - _start), intersect(_end), brush(nullptr), face(nullptr) { }

// TODO: Clean this up and make raytest_face use it
bool raytest_plane(dy_bplane* plane, dy_ray* ray)
{
	// (r * t + o) . n = d
	// t * r . n + o . n = d
	// t = ( d - o . n ) / (r . n )

	float approach = plane->norm.dot(ray->dir);

	// Approaching from the back?
//	if (approach >= 0)
//		return false;

	// Parametric intersection
	float t = (plane->d - ray->start.dot(plane->norm)) / approach;

	// Dump backwards, too far, and too large t's
	if (t < 0 || t > 1 || t >= ray->t)
		return false;

	// Store results
	ray->intersect = ray->start + ray->dir * t;
	ray->t = t;
	
	return true;
}

bool raytest_face(dy_rface* face, dy_ray* ray)
{
	// (r * t + o) . n = d
	// t * r . n + o . n = d
	// t = ( d - o . n ) / (r . n )

	float approach = face->plane->norm.dot(ray->dir);

	// Approaching from the back?
	if (approach >= 0)
		return false;

	// Parametric intersection
	float t = (face->plane->d - ray->start.dot(face->plane->norm)) / approach;

	// Dump backwards, too far, and too large t's
	if (t < 0 || t > 1 || t >= ray->t)
		return false;

	vec3 p = ray->start + ray->dir * t;


	dy_rhalfedge* he = face->first;
	do {

		// TODO: Optimize this!
		vec3 start = *he->vert->pos;
		vec3 end   = *he->next->vert->pos;

		vec3 tang = end - start;

		vec3 bi = tang.cross(face->plane->norm);

		float d = bi.dot(p - start);

		if (d > 0)
			return false;

		he = he->next;
	} while (he != face->first);

	ray->t = t;
	ray->intersect = p;
	return true;
}

// Slab AABB ray test
bool raytest_aabb(dy_ray* ray, dy_aabb aabb)
{
	aabb.mins -= {1, 1, 1};
	aabb.maxs += {1, 1, 1};

	
	vec3 tn = vec3::ew_div(aabb.mins - ray->start, ray->dir);
	vec3 tx = vec3::ew_div(aabb.maxs - ray->start, ray->dir);

	vec3 tmin = vec3::min(tn, tx);
	vec3 tmax = vec3::max(tn, tx);

	return tmin.max() <= tmax.min();
}


extern dy_netdb_tree s_tree;
void raytest(dy_ray* ray)
{
	dy_ustack<dy_netdb_obj*>& solidobjs = s_tree.objectsForType(DY_NETDB_TYPE_SOLID);
	dy_ustack<dy_netdb_objref<dy_brush>>* solids = (dy_ustack<dy_netdb_objref<dy_brush>>*)&solidobjs;


	for (dy_netdb_objref<dy_brush>* bref : *solids)
	{
		dy_brush* b = bref->ref();

		dy_rmesh* mesh = &b->mesh;
		for (int i = 0; i < mesh->face_count; i++)
		{
			if (!raytest_aabb(ray, mesh->aabb))
				continue;

			dy_rface* f = mesh->faces[i];
			if (raytest_face(f, ray))
			{
				ray->face = f;
				ray->brush = *bref;
			}
		}
	}
	//dy_doodle_aabb(b->mesh.aabb, vec4{ 1,0,0,0.5 }, 0, -1);
	
}



// ------------------ //
//     World Mesh     //
// ------------------ //

vec2 mesh_caluv(vec3 norm, vec3 absnorm, vec3 pos, vec4 txdata)
{
	vec2 uv;
	if (absnorm.x >= absnorm.z && absnorm.x >= absnorm.y)
	{
		uv = pos.zy();
		uv.x *= norm.x > 0.0 ? 1.0 : -1.0;
	}
	else if (absnorm.z >= absnorm.y && absnorm.z >= absnorm.x)
	{
		uv = pos.xy();
		uv.x *= norm.z < 0.0 ? 1.0 : -1.0;
	}
	else
	{
		uv = pos.xz();
		uv.x *= norm.y < 0.0 ? -1.0 : 1.0;
	}
	uv.y *= -1.0;

	uv.x = uv.x * txdata.z + txdata.x;
	uv.y = uv.y * txdata.w + txdata.y;

	// FIXME: 
	uv.x *= -1;

	return uv;
}