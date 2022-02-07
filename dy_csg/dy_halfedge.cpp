#include "dy_halfedge.h"
#include "dy_brush.h"
#include <stdlib.h>
#include <assert.h>


void dy_rmesh_clear(dy_rmesh* mesh)
{
	free(mesh->points);
	if (mesh->faces)
	{
		for (int i = 0; i < mesh->face_count; i++)
		{
			dy_rface* face = mesh->faces[i];
			if (!face)
				continue;

			dy_rhalfedge* first = face->first, * he = first, *n = 0;
			do
			{
				n = he->next;
				free(he);
				he = n;
			} while (he != first);


			free(face);
		}
		free(mesh->faces);
	}
}

// These exist so that when I get the managed allocator going, it's less effort to make functional
dy_rhalfedge* dy_rhe_new_()
{
	return (dy_rhalfedge*)malloc(sizeof(dy_rhalfedge));
}
dy_rvertex* dy_rvt_new_()
{
	return (dy_rvertex*)malloc(sizeof(dy_rvertex));
}


//     lower //\ upper     \//\
//          //             ..     <- split
//         //             //\
// upper \// lower      \//
//       
void dy_split_edge_(dy_rhalfedge* edge, vec3* p)
{
	// Split the edge
	dy_rhalfedge* upper = dy_rhe_new_();
	upper->face = edge->face;
	upper->next = edge->next;
	
	dy_rvertex* mid = dy_rvt_new_();
	mid->edge = upper;
	mid->pos = p;

	upper->vert = mid;

	edge->next = upper;

	// Check if we have a pair
	dy_rhalfedge* pair = edge->pair;
	if (pair)
	{
		// Split the pair
		dy_rhalfedge* pairupper = dy_rhe_new_();
		pairupper->face = pair->face;
		pairupper->next = pair->next;

		dy_rvertex* pairmid = dy_rvt_new_();
		mid->edge = pairupper;
		mid->pos = p;

		pairupper->vert = pairmid;

		pair->next = pairupper;
		
		// Attach the pairs
		edge->pair = pairupper;
		upper->pair = pair;
	}

}

enum
{
	DY_HE_UNKNOWN,
	DY_HE_INFRONT,
	DY_HE_BEHIND,
	DY_HE_EXITS,
	DY_HE_ENTERS,
};

// a: Origin t, b: End t
// Edge plane intersection
// 
// Cases
//   a>0 b>0 : infront of plane
//   a<0 b<0 : behind plane
//   a<0 b>0 : edge enters plane
//   a>0 b<0 : edge exits plane
//   a=0 b>0 : edge stems out of plane
//   a=0 b<0 : edge stems into plane
//   a>0 b=0 : edge ends at front of plane
//   a<0 b=0 : edge ends at back of plane
//   a=0 b=0 : edge entirely on plane
static int dy_classify_(float a, float b, int last)
{
	// There's probably a great way to melt this down into an array, bitflags, or something speedy
	// I cannot be bothered

	if (b > 0)
	{
		if (a > 0)
			return DY_HE_INFRONT;
		if (a < 0)
			return DY_HE_ENTERS;

		// a == 0	
		if (last == DY_HE_BEHIND)
			return DY_HE_ENTERS;
		return DY_HE_INFRONT;
	}
	
	if (b < 0)
	{
		if (a < 0)
			return DY_HE_BEHIND;
		if (a > 0)
			return DY_HE_EXITS;
		
		// a == 0	
		if (last == DY_HE_INFRONT)
			return DY_HE_EXITS;
		return DY_HE_BEHIND;
	}


	// b == 0
	if (a > 0)
		return DY_HE_INFRONT;
	if (a < 0)
		return DY_HE_BEHIND;
	return last;
}

struct dy_rface_plane_intersect
{
	dy_rhalfedge* enter;
	dy_rhalfedge* exit;
	vec3 enterpoint;
	vec3 exitpoint;
};

dy_rface_plane_intersect dy_rface_intersect_(dy_rface* face, dy_bplane* plane)
{
	dy_rhalfedge* first = face->first, *he = first, *n;

	dy_rface_plane_intersect intersect = {0,0};

	dy_rvertex* vorigin = he->vert, *vend;
	int leclass = DY_HE_UNKNOWN;
	do
	{
		n = he->next;
		vend = n->vert;
		
		vec3 origin = *vorigin->pos;
		vec3 end = *vend->pos;

		float od = origin.dot(plane->norm);
		float ed = end.dot(plane->norm);

		float a = plane->d - od;
		float b = plane->d - ed;

		leclass = dy_classify_(a, b, leclass);

		if (leclass == DY_HE_ENTERS)
		{
			intersect.enter = he;
			
			// Get the intersection
			assert(ed - od != 0.0f);
			float t = a / (ed - od);
			intersect.exitpoint = (1.0f - t) * origin + t * end;

			// Done when both are found
			if (intersect.exit)
				return intersect; 
		}
		else if (leclass == DY_HE_EXITS)
		{
			intersect.exit = he;

			// Get the intersection
			assert(ed - od != 0.0f);
			float t = a / (ed - od);
			intersect.exitpoint = (1.0f - t) * origin + t * end;

			// Done when both are found
			if (intersect.enter)
				return intersect;
		}
		origin = end;
		he = n;
	}
	while (he != first);


	return { 0,0, {0,0,0}, {0,0,0} };
}

// ((1-t)o + (t)e) . n = d
// (1-t)o . n + (t)e . n  = d
// o . n - t(o . n) + (t)e . n  = d
// t(e . n - o . n)  = d - o . n
// t = (d - o . n) / (e . n - o . n)

void dy_rface_cut(dy_rface* face, dy_bplane* plane)
{

}


