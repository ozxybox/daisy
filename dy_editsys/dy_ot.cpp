#include "dy_ot.h"
#include <unordered_map>
#include "xstring.h"

// FIXME: REMOVE THIS
extern dy_netdb_tree s_tree;


class dy_ot_opfactory
{
public:
	virtual dy_ot_operator* create() = 0;
};

template<typename T>
class _dy_ot_opfactory : public dy_ot_opfactory
{
public:
	virtual dy_ot_operator* create() override
	{
		return new T();
	}
};


// TODO: Separate this from s_otopdict
#define DECLARE_OPERATOR(classname, printname) \
	{XSTRL(#printname), new _dy_ot_opfactory<classname>()},


static std::unordered_map<xstr_t, dy_ot_opfactory*, xstrhash_t, xstrequality_t> s_otopdict =
{
	DECLARE_OPERATOR(dy_ot_operator_delete,       delete)
	DECLARE_OPERATOR(dy_ot_operator_create_solid, create_solid)
	DECLARE_OPERATOR(dy_ot_operator_create_plane, create_plane)
	DECLARE_OPERATOR(dy_ot_operator_update_plane, update_plane)

};

dy_ot_operator* dy_ot_create_operator(xstr_t name)
{
	auto f = s_otopdict.find(name);
	if (f == s_otopdict.end())
		return 0;
	return f->second->create();
}


// Delete
void dy_ot_operator_delete::apply(void* data)
{
}

void dy_ot_operator_delete::commit()
{
}

void dy_ot_operator_delete::revert()
{
}


// Create Plane
void dy_ot_operator_create_plane::apply(void* data)
{
}

void dy_ot_operator_create_plane::commit()
{
}

void dy_ot_operator_create_plane::revert()
{
}

// Create Solid
void dy_ot_operator_create_solid::apply(void* data)
{
}

void dy_ot_operator_create_solid::commit()
{
}

void dy_ot_operator_create_solid::revert()
{
}

// Update Plane
void dy_ot_operator_update_plane::apply(void* data)
{
	dy_ot_data_update_plane* d = (dy_ot_data_update_plane*)data;
	dy_netdb_obj* obj = s_tree.find(d->target);
	void* p = obj->ptr();
	if (obj->type() != DY_NETDB_TYPE_PLANE || !p)
	{
		// TODO: Return error
		return;
	}

	*(dy_bplane*)p = d->content;
}

void dy_ot_operator_update_plane::commit()
{
}

void dy_ot_operator_update_plane::revert()
{
}
