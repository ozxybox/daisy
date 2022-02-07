#include "dy_csg.h"
#include "dy_brush.h"


enum DY_CSG_OPERATION
{
	DY_CSG_NONE,         // Solid left as is
	DY_CSG_UNION,        // Merge solids
	DY_CSG_INTERSECTION, // Intersection of solids
	DY_CSG_DIFFERENCE    // Subtract solids
};

struct dy_csgnode
{
	DY_CSG_OPERATION operation;

	union
	{
		dy_bsolid* solid; // When operation is NONE
		struct
		{
			dy_csgnode* left;
			dy_csgnode* right;
		};
	};
};

struct dy_csgtree
{
	dy_csgnode* top;
};
