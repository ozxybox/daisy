#include "dy_brush.h"
#include "dy_halfedge.h"
#include "dy_ustack.h"
#include <math.h>
#include <stdio.h>
#include <assert.h>

// Rules of the brush
//   1) This isn't a hull. We're finding the smallest shape that fits within these planes
//   2) Two planes cannot share more than one edge
//   3) One point can be shared by N edges (imagine a cone)
//   4) Different planes can come together to make the same point (checkerboard of quads)
//   5) 3 plane intersection -> 1 point and 3 incomplete edges


//#define BRUSH_LOG printf
#define BRUSH_LOG(...) (0)


// Brush Edge
// This is a template for creating a half edge
struct dy_bedge
{
	dy_rhalfedge* he;
	int pair_plane;
	int v[2];
	int found; // how many verts of this edge have been found?
	dy_bedge* pair;
};

// Brush Face
struct dy_bface
{
	int valid_edges;
	dy_ustack<dy_bedge> edges;
	dy_bplane* plane;
	dy_rface* face;
};

#if 0
bool dy_inside(dy_bsolid* solid, dy_rface* face)
{
	int inside = 0;
	int outside = 0;

	for (int i = 0; i < solid->plane_count; i++)
	{
		dy_bplane* plane = &solid->planes[i];

		dy_rhalfedge* first = face->first, * he = first;
		do
		{
			vec3* p = he->vert->pos;

			float d = p->dot(plane->norm) - plane->d;

			if (d >= 0)
				return false;


			he = he->next;
		} while (he != first);
	}

	return true;
}
#endif

bool dy_bplane_intersect(dy_bplane* plane1, dy_bplane* plane2, dy_bplane* plane3, vec3* out)
{
	vec3 n1 = plane1->norm;
	vec3 n2 = plane2->norm;
	vec3 n3 = plane3->norm;

	// Push all norms into the rows
	mat3 n = 
	{{n1.x, n2.x, n3.x},
	 {n1.y, n2.y, n3.y},
	 {n1.z, n2.z, n3.z}};

	vec3 d = { plane1->d, plane2->d, plane3->d };

	// Solve x for n*x=d
	vec3 i;
	if (n.solve(d, &i))
	{
		*out = i;
		return true;
	}

	return false;
}

// Utils for tracking new points and edges
// TODO: Optimize these!
// TODO: Change point similarity detection to use more metrics like brush size, plane distance, and etc
// TODO: Would it be better to allow for many edges to get submitted for a face, and then to resolve down those edges into just a few?
//       It might allow us to look at the results, see how close certain points are, and resolve more overlapping points
//       dy_bsolid_mesh_submit_point_ keeps causing issues in dy_bsolid_mesh_push_edge_point_ due to incorrectly resolving points
//       This still wouldn't solve issues with 4 planes making one point though...
dy_bedge* dy_bsolid_mesh_get_edge_(dy_ustack<dy_bedge>& elist, int pair_plane)
{
	for (dy_bedge* oe : elist)
	{
		if (oe->pair_plane == pair_plane)
			return oe;
	}

	return elist.push({ 0,pair_plane,{0,0},0,0 });
}
void dy_bsolid_mesh_push_edge_point_(dy_bedge* edge, int pidx)
{
	// Check if we've got this point already
	if (edge->found > 0)
	{
		if (edge->v[0] == pidx)
		{
			BRUSH_LOG("Already tracking! ");
			return;
		}
		if (edge->found > 1 && edge->v[1] == pidx)
		{
			BRUSH_LOG("2_Already_tracking_2 ");
			return;
		}
	}

	assert(edge->found < 2); // If this fires, it's likely that 2 points were mistakenly found to be distinct. Look at dy_bsolid_mesh_submit_point_

	// Don't have it recorded yet. Pop it in
	edge->v[edge->found] = pidx;
	edge->found++;
}
dy_bedge* dy_bsolid_mesh_push_plane_edge_(dy_ustack<dy_bedge>& elist, int pair_plane, int pidx)
{
	for(dy_bedge* oe : elist)
	{
		if (oe->pair_plane == pair_plane)
		{
			dy_bsolid_mesh_push_edge_point_(oe, pidx);
			return oe;
		}
	}
	return elist.push({ 0,pair_plane,{pidx,0},1,0 });
}
int dy_bsolid_mesh_submit_point_(dy_ustack<vec3, 8>& pointlist, vec3& p)
{
	// TODO: Tune the epsilon for this!
	const float epsilon = 0.005f;
	// FIXME: This should be a hashmap! Or literally anything else! This is abysmal!
	// Hunt and seek out any points of the same value
	int i = 0;
	for(vec3* v : pointlist)
	{
#if 1
		if (dy_near(p.x, v->x, epsilon) && dy_near(p.y, v->y, epsilon) && dy_near(p.z, v->z, epsilon))
#else
		if(dy_near((*v - p).mag(), 0, epsilon))
#endif
		{
			BRUSH_LOG("Point match!! ");
			return i;
		}
		i++;
	}
	
	assert(i == pointlist.count);
	// At this point i is pointlist.count;
	pointlist.push(&p);
	return i;

}

// Convert a Brush Solid into a halfedge mesh
dy_rmesh dy_bsolid_mesh(dy_bsolid* solid)
{
	// We need at least four planes to make anything 
	if (solid->plane_count < 4)
		return {0,0,0};


	dy_ustack<vec3, 8> pointlist;

	// Hold and array of what planes have been okay'd or are invalid
	bool* plane_valid = (bool*)malloc(sizeof(bool) * solid->plane_count);
	memset(plane_valid, 1, sizeof(bool) * solid->plane_count);

	// First, see if there's any full planes we can discard
	int cplane = solid->plane_count;
	dy_ustack<dy_bface> bfaces;
	for (int i = 0; i < cplane; i++)
	{
		// Skip invalid planes
		if (plane_valid[i] == false)
			continue;

		dy_bplane* plane = &solid->planes[i];
		
		for (int k = i + 1; k < cplane; k++)
		{
			dy_bplane* otherplane = &solid->planes[k];

			float d = vec3::dot(plane->norm, otherplane->norm);
			if (dy_near(d, 1.0f))
			{
				// Parallel!
				if (plane->d > otherplane->d)
				{
					// Infront! We're invalid!
					plane_valid[i] = false;
					break;
				}
				else
				{
					// Behind! Mark the plane infront of us invalid
					plane_valid[k] = false;
				}
			}

		}

	}

	// FIXME: use an array instead of a ustack for bfaces!
	for (int i = 0; i < solid->plane_count; i++)
	{
		if (plane_valid[i])
			bfaces.push({ 0,{},&solid->planes[i],0 });
	}
	free(plane_valid);
	int face_count = bfaces.count;


	// We need at least four planes to make anything 
	if (face_count < 4)
		return { 0,0,0 };


	// Find intersection points, cull them, record them, record their edges
	dy_ustack_iterator<dy_bface> xit = bfaces.begin();
	for (int x = 0; x < bfaces.count; x++, ++xit)
	{
		dy_bface* xface = *xit;
		dy_bplane* xplane = xface->plane;
		dy_ustack<dy_bedge>& xelist = xface->edges;

		// Loop for each plane that we haven't tested yet
		dy_ustack_iterator<dy_bface> yit = xit + 1;
		for (int y = x + 1; y < bfaces.count; y++, ++yit)
		{
			dy_bface* yface = *yit;
			dy_bplane* yplane = yface->plane;
			dy_ustack<dy_bedge>& yelist = yface->edges;

			// Are X & Y opposing and parallel? If so, there will never be a intersection
			// No need to check for same dir and parallel, we discarded those earlier

			// TODO: use distance between plane d's to change the epsilon
			//		 Two planes that intersect 10 km away are not intersecting
			//       investigate dot vs cross for this
#if 1
			float f = xplane->norm.dot(yplane->norm);
			if (dy_near(f, -1.0))
				continue;
#else
			float f = vec3::cross(xplane->norm, yplane->norm).mag();
			if (dy_near(f, 0, 0.001f))
				continue;
#endif
			


			dy_bedge* xedge = 0;
			dy_bedge* yedge = 0;

			// Planes X and Y share an edge
			// Loop for the other remaining untested planes
			dy_ustack_iterator<dy_bface> zit = yit + 1;
			for (int z = y + 1; z < bfaces.count; z++, ++zit)
			{
				dy_bface* zface = *zit;
				dy_bplane* zplane = zface->plane;
				dy_ustack<dy_bedge>& zelist = zface->edges;
				
				// Do our 3 planes intersect? If not, next plane.
				vec3 p;
				if (!dy_bplane_intersect(xplane, yplane, zplane, &p))
					continue;
				// Hit!
					
				// Is it within this solid?
				bool in = true;
				for(dy_bface* k : bfaces)
				{
					if (k == xface || k == yface || k == zface)
						continue;

					dy_bplane* plane = k->plane;
					if (vec3::dot(p, plane->norm) - plane->d > DY_EPSILON)
					{
						in = false;
						break;
					}
				}

				// Out of the solid. Skip it
				if (!in)
					continue;

				// This one's clean! Push it
				int pidx = dy_bsolid_mesh_submit_point_(pointlist, p);

				// Planes X Y and Z share a point
				// Each plane gains 2 incomplete half edges

				// Do we have these edges yet?
				if (!xedge || !yedge)
				{
					xedge = dy_bsolid_mesh_get_edge_(xelist, y);
					yedge = dy_bsolid_mesh_get_edge_(yelist, x);
					xedge->pair = yedge;
					yedge->pair = xedge;
				}

				// Add the point to our x & y shared edge
				dy_bsolid_mesh_push_edge_point_(xedge, pidx);
				dy_bsolid_mesh_push_edge_point_(yedge, pidx);

				// Planes X and Y also gain a z edge
				dy_bedge* xze = dy_bsolid_mesh_push_plane_edge_(xelist, z, pidx);
				dy_bedge* yze = dy_bsolid_mesh_push_plane_edge_(yelist, z, pidx);

				// Plane Z gains two edges
				dy_bedge* zxe = dy_bsolid_mesh_push_plane_edge_(zelist, x, pidx);
				dy_bedge* zye = dy_bsolid_mesh_push_plane_edge_(zelist, y, pidx);

				xze->pair = zxe;
				zxe->pair = xze;

				yze->pair = zye;
				zye->pair = yze;


				// No two planes can share more than one edge, so if found is 2, we stop
				if (xedge->found >= 2 || yedge->found >= 2)
				{
					BRUSH_LOG("escape early!");
					break;
				}

			}
		}

	}

	// Need at least 4 points for a brush
	if(pointlist.count < 4)
		return { 0,0,0 };


	// Now that we have a bunch of potential edges, let's toss out the invalids
	for (dy_bface* face : bfaces)
	{
		// Need at least 3 edges for a valid face!
		if (face->edges.count < 3)
			continue;

		for (dy_bedge* edge : face->edges)
		{
			assert(edge->found < 3);
			// Skip valid edges
			if (edge->found == 2 && edge->v[0] != edge->v[1])
			{
				face->valid_edges++;
				continue;
			}

			// Skip edges already tagged invalid
			if (edge->found == -1)
				continue;

#if 0
			if (edge->found == 2 && edge->v[0] == edge->v[1])
				BRUSH_LOG("Double v point! ");
			if (edge->found != 2)
				BRUSH_LOG("Incomplete edge! ");
#endif 

			// Invalid edge! Mark it as such
			edge->found      = -1;
			edge->pair_plane = -1;
			edge->v[0]       = -1;
			edge->v[1]       = -1;
				
			dy_bedge* pair = edge->pair;
			if (pair)
			{
				// Invalidate the pair as well just in case
				pair->found      = -1;
				pair->pair_plane = -1;
				pair->v[0]       = -1; 
				pair->v[1]       = -1;
					
				pair->pair = 0;
				edge->pair = 0;
			}
		}
	}

	vec3* points = pointlist.packed();

	
	// Assemble faces
	for(dy_bface* brush_face : bfaces)
	{
		// Do we have enough valid edges for a face?
		if (brush_face->valid_edges < 3)
			continue;
		
		dy_ustack<dy_bedge>& elist = brush_face->edges;
		dy_bplane* plane = brush_face->plane;

		// Create a new HE face for this brush face
		dy_rface* face = (dy_rface*)malloc(sizeof(dy_rface));
		face->plane = plane;
		brush_face->face = face;

		// Prep the first edge
		dy_rhalfedge* he = (dy_rhalfedge*)malloc(sizeof(dy_rhalfedge)), *first_he = he;
		dy_rvertex* vt = (dy_rvertex*)malloc(sizeof(dy_rvertex));
		he->face = face;
		he->vert = vt;
		he->pair = 0;
		vt->edge = he;
		
		face->first = first_he;


		// Find our first valid edge
		dy_bedge* e = 0;
		auto itstart = elist.begin();
		auto itend = elist.end();
		for (; itstart != itend; ++itstart)
		{
			dy_bedge* be = *itstart;
			if (be->found == 2)
			{
				e = be;
				break;
			}
		}
		assert(e);
		assert(e->found == 2);
		
		// Attach our discovery to our edge
		e->he = he;
		

		int lastpoint = -1;
		vec3* conp, * ep;

		// Loop all edges and find the connections
		// TODO: Split the lastpoint == -1 paths off into an initial loop?
		// itstart + 1 so that we skip the first edge. We already have it above 
		for (auto it = itstart + 1; it != itend; ++it)
		{
			dy_bedge* oe = *it;
			if (oe == e || oe->he != 0 || oe->found != 2)
				continue;

			int oei = 0;
			
			if (lastpoint == -1)
			{
				// No last point. Let's find a connection
				int ei = 0;
				for (; ei < 2; ei++)
					for (oei = 0; oei < 2; oei++)
						if (e->v[ei] == oe->v[oei])
						{
							goto fullBreak;
						}
				continue;
				fullBreak:

				// point of connection
				conp = &points[e->v[ei]];

				// The other points on the edges
				ep = &points[e->v[(ei + 1) % 2]];
			}
			else
			{
				// We had a last point, we can use it as our connector
				bool connected = false;
				for (; oei < 2; oei++)
					if (lastpoint == oe->v[oei])
					{
						connected = true;
						break;
					}
				if (!connected)
					continue;

			}

			// These edges are connected!

			// Get the other point on the connected line
			int oepidx = oe->v[(oei + 1) % 2];
			vec3* oep = &points[oepidx];


			if (lastpoint == -1)
			{
				// For our first edge, we need to establish the winding order
				float n = vec3::dot(vec3::cross(*conp - *ep, *oep - *conp), plane->norm);
				if (n <= 0)
				{
					// Either not the same direction or 0 area!
					continue;
				}
			}

			// New edge 
			dy_rhalfedge* ohe = (dy_rhalfedge*)malloc(sizeof(dy_rhalfedge));
			dy_rvertex* ovt = (dy_rvertex*)malloc(sizeof(dy_rvertex));
			ohe->face = face;
			ohe->vert = ovt;
			ohe->pair = 0;
			ovt->edge = ohe;
			ovt->pos = conp;
			oe->he = ohe;

			// Stitch it onto our edge!
			he->next = ohe;
			if(lastpoint == -1)
				vt->pos = ep;

			// Set the current edge
			he = ohe;
			vt = ovt;
			e = oe;

			// Mark where our next connector will be
			lastpoint = oepidx;
			ep = conp;
			conp = oep;

			// Back to the start!
			// The loop'll increase this in a moment, so no + 1
			it = itstart;
		}


		if (lastpoint == -1)
		{
			// If we get to here, we failed to create a proper face
			// It might be a 0 area poly?
			BRUSH_LOG("failed to assemble face! ");

			// Clean it up
			free(he);
			free(vt);
			free(face);
			e->he = 0;

			brush_face->valid_edges = 0;
			
			continue;
		}

		assert(first_he != he);

		// Link the end back to the start
		he->next = first_he;


	}

	// Stitch the faces together at the edges
	int valid_faces = 0;
	for(dy_bface* face : bfaces)
	{
		dy_ustack<dy_bedge>& elist = face->edges;

		// Do we have enough edges for a face?
		if (face->valid_edges < 3 || !face->face || !face->face->first)
			continue;

		for(dy_bedge* e : elist)
		{
			if (e->found != 2) continue;
			if (!e->he) continue;
			if (!e->he->pair) continue;
			assert(e->he);
			assert(e->pair->he);

			e->he->pair = e->pair->he;
			e->pair->he->pair = e->he;
		}

		valid_faces++;
	}

	BRUSH_LOG("woo hoo!");

	dy_rmesh mesh;
	mesh.points = points;
	mesh.faces = (dy_rface**)malloc(sizeof(dy_rface*) * valid_faces);
	mesh.face_count = valid_faces;

	int faceidx = 0;
	for (dy_bface* face : bfaces)
	{
		// Do we have enough edges for a face?
		if (face->valid_edges < 3 || !face->face || !face->face->first)
			continue;

		mesh.faces[faceidx] = face->face;

		faceidx++;
	}

	
	return mesh;
}

#undef BRUSH_LOG