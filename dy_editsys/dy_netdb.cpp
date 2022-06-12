#include "dy_netdb.h"
#include "dy_ot.h"

#define ROOT_ID ((dy_netdb_id)0u)




dy_netdb_tree::dy_netdb_tree() : m_idserial(0)
{
	m_typelist = new dy_ustack<dy_netdb_obj*>[DY_NETDB_TYPE_COUNT];

	// This is the root. It is created purely for other objects to parent to
	create(0, DY_NETDB_TYPE_NONE, nullptr);


}

dy_netdb_obj* dy_netdb_tree::find(dy_netdb_id id)
{
	auto f = m_objects.find(id);

	if (f == m_objects.end())
		return 0;

	return f->second;
}

dy_netdb_obj* dy_netdb_tree::create(dy_netdb_id parent, dy_netdb_type type, void* ptr)
{
	assert(type >= 0 && type < DY_NETDB_TYPE_COUNT);

	dy_netdb_id id;
	do {
		id = m_idserial++;
	} while (m_objects.find(id) != m_objects.end() && m_deletedobjects.find(id) != m_deletedobjects.end());


	return forceCreate(id, parent, type, ptr);
}

dy_netdb_obj* dy_netdb_tree::forceCreate(dy_netdb_id id, dy_netdb_id parent, dy_netdb_type type, void* ptr)
{
	assert(type >= 0 && type < DY_NETDB_TYPE_COUNT);

	dy_netdb_obj* entry = new dy_netdb_obj;
	entry->m_id     = id;
	entry->m_parent = parent;
	entry->m_type   = type;
	entry->m_ptr    = ptr;
	
	
	// Track the entry
	m_objects.insert({ id, entry });
	m_tree.insert({ id, {} });
	
	m_tree[parent].push_back(entry);


	// Track the entry for its type
	m_typelist[type].push(entry);
	
	return entry;
}

const dy_array<dy_netdb_obj*> dy_netdb_tree::children(dy_netdb_id id)
{
	auto f = m_tree.find(id);
	if (f == m_tree.end())
		return { 0,0 };

	std::vector<dy_netdb_obj*>& vec = f->second;
	return { (unsigned int)vec.size(), vec.data() };
}

void dy_netdb_tree::rebuild()
{
	/*
	m_typelist->clear();
	for (auto& p : m_objects)
	{
		m_typelist[p.second->m_type].push(p.second);
	}
	*/

	m_tree.clear();

	// Populate the tree with all objects
	for (auto& p : m_objects)
		m_tree.insert({ p.second->m_id, {}});

	// Fill in the parents with the children's IDs
	for (auto& p : m_objects)
	{
		m_tree[p.second->m_parent].push_back(p.second);
	}

}

unsigned int dy_netdb_tree::objectCount()
{
	return m_tree.size();
}

dy_netdb_obj* dy_netdb_tree::objectAtIndex(unsigned int index)
{
	auto f = m_objects.begin();
	std::advance(f, index);
	return f->second;
}


dy_ustack<dy_netdb_obj*>& dy_netdb_tree::objectsForType(dy_netdb_type type)
{
	assert(type >= 0 && type < DY_NETDB_TYPE_COUNT);

	return m_typelist[type];
}

#if 0
dy_bsolid* dy_ndb_newsolid()
{
	dy_bsolid* solid = new dy_bsolid;

	solid->planes = new dy_bplane[]{
		{{ 0, 1, 0}, 32}, // Up
		{{ 0,-1, 0}, 32}, // Down
		{{ 1, 0, 0}, 32}, // Right
		{{-1, 0, 0}, 32}, // Left
		{{ 0, 0, 1}, 32}, // Back
		{{ 0, 0,-1}, 32}, // Front
	};
	solid->plane_count = 6;

	//solid->id = dy_ndb_newid(WORLD_ID, NDB_TYPE_SOLID, solid);
	//for (int i = 0; i < 6; i++)
	//	solid->planes[i].id = dy_ndb_newid(solid->id, NDB_TYPE_PLANE, &solid->planes[i]);

	return solid;
}

dy_bsolid* dy_ndb_getsolid(dy_ndb_id id)
{
	auto f = s_meshdb.find(id);
	if (f == s_meshdb.end())
		return 0;
	if (f->second->type == NDB_TYPE_SOLID)
		return static_cast<dy_bsolid*>(f->second->ptr);
	return 0;
}

dy_bplane* dy_ndb_getplane(dy_ndb_id id)
{
	auto f = s_meshdb.find(id);
	if (f == s_meshdb.end())
		return 0;
	if (f->second->type == NDB_TYPE_PLANE)
		return static_cast<dy_bplane*>(f->second->ptr);
	return 0;
}

dy_bsolid* dy_ndb_parentsolid(dy_ndb_id id)
{
	auto f = s_meshdb.find(id);
	if (f == s_meshdb.end())
		return 0;
	if (f->second->type == NDB_TYPE_PLANE)
		return dy_ndb_getsolid(f->second->parent);
	return 0;
}

#endif


dy_ot_history::dy_ot_history()
	: m_stampserial(0)
{

}

dy_ot_record* dy_ot_history::create()
{
	dy_ot_record* rec = new dy_ot_record;
	rec->stamp = m_stampserial++;
	return rec;
}


void dy_ot_history::commit(dy_ot_record* rec)
{
	// TODO: Check for failure
	rec->op->commit();
	m_timeline.push(rec);
}

void dy_ot_history::undo()
{
	dy_ot_record* rec = *m_timeline.last();
	m_timeline.pop();
	rec->op->revert();
	m_undone.push(rec);
}
void dy_ot_history::redo()
{
	dy_ot_record* rec = *m_undone.last();
	m_undone.pop();
	// TODO: Check for failure
	rec->op->commit();
	m_timeline.push(rec);
}