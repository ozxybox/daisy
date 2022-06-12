#pragma once
#include "dy_array.h"
#include "dy_ustack.h"
#include <unordered_map>
#include <vector>

typedef unsigned int dy_netdb_id;
typedef unsigned int dy_ot_stamp;

typedef unsigned int dy_netdb_type;
enum
{
	DY_NETDB_TYPE_NONE,
	DY_NETDB_TYPE_DATATABLE,
	DY_NETDB_TYPE_WORLD,
	DY_NETDB_TYPE_SOLID,
	DY_NETDB_TYPE_PLANE,
	DY_NETDB_TYPE_TEXT,

	DY_NETDB_TYPE_COUNT,
};

class dy_netdb_tree;


// TODO: When we get to storing all of this, don't store parent--just attach it as an array of children
//       We don't do that when live for sanity reasons
class dy_netdb_obj
{
public:
	dy_netdb_id   id()      { return m_id;      }
	dy_netdb_id   parent()  { return m_parent;  }
	dy_netdb_type type()    { return m_type;    }
	dy_ot_stamp   version() { return m_version; }
	void* ptr() { return m_ptr; }

private:
	dy_netdb_id   m_id;
	dy_netdb_id   m_parent;
	dy_netdb_type m_type;
	dy_ot_stamp   m_version;
	void* m_ptr;

	
	friend class dy_netdb_tree;
};


template<typename T>
class dy_netdb_idref
{
public:

	dy_netdb_idref(dy_netdb_id id)
		: m_id(id) {};

	dy_netdb_id id() const {
		return m_id;
	}

	T* operator ->() const {
		return s_tree.find(m_id)->ptr();
	}

	operator T*() const {
		return s_tree.find(m_id)->ptr();
	}

private:
	const dy_netdb_id m_id;
};


template<typename T>
class dy_netdb_objref
{
public:

	dy_netdb_objref(dy_netdb_obj* obj)
		: m_obj(obj) {};
	dy_netdb_objref(dy_netdb_objref<T>&& obj)
		: m_obj(obj.m_obj) {};
	dy_netdb_objref(dy_netdb_objref<T>& obj)
		: m_obj(obj.m_obj) {};


	dy_netdb_obj* obj() const {
		return m_obj;
	}

	T* ref() const {
		return (T*)m_obj->ptr();
	}

	dy_netdb_id id() const {
		return m_obj->id();
	}

	dy_netdb_objref<T>& operator =(const dy_netdb_objref<T>& l) {
		m_obj = l.m_obj;
		return *this;
	}

	T* operator ->() const {
		return (T*)m_obj->ptr();
	}

	operator T*() const {
		return (T*)m_obj->ptr();
	}

	operator dy_netdb_idref<T> () const {
		return dy_netdb_idref(m_obj->id());
	}

private:
	dy_netdb_obj* m_obj;
};




// This tree represents all objects and creates their hierarchy from their parent ids
class dy_netdb_tree
{
public:
	dy_netdb_tree();

	void rebuild();

	dy_netdb_obj* find(dy_netdb_id id);
	dy_netdb_obj* create(dy_netdb_id parent, dy_netdb_type type, void* ptr);
	dy_netdb_obj* forceCreate(dy_netdb_id id, dy_netdb_id parent, dy_netdb_type type, void* ptr);

	const dy_array<dy_netdb_obj*> children(dy_netdb_id id);

	unsigned int objectCount();
	dy_netdb_obj* objectAtIndex(unsigned int index);

	/*const*/ dy_ustack<dy_netdb_obj*>& objectsForType(dy_netdb_type type);


//private:


	dy_netdb_id m_idserial;

	// All objects mapped with their ids
	// TODO: Mush the key into the value, with the map utilizing the value's data's key
	std::unordered_map<dy_netdb_id, dy_netdb_obj*> m_objects;
	
	// Flat represensation of the tree's hierarchy
	// Look up ID, get children, select child, lookup ID, repeat
	std::unordered_map<dy_netdb_id, std::vector<dy_netdb_obj*>> m_tree;


	// All objects that have been removed
	// TODO: Mush the key into the value, with the map utilizing the value's data's key
	std::unordered_map<dy_netdb_id, dy_netdb_obj*> m_deletedobjects;

	// List of all objects by type
	dy_ustack<dy_netdb_obj*>* m_typelist;
};

class dy_ot_operator;
struct dy_ot_record
{
	// Stamps are the dy_netdb_id of ot. The 
	dy_ot_stamp stamp;

	// Who created this record?
// author

	// What does this operator depend on (or modify)
	dy_array<dy_netdb_id> dependencies;

	// The actual operator
	dy_ot_operator* op = 0;
};


#if 0
// An undone lost segment of time
struct dy_ot_timeline_fragment
{
	dy_ot_stamp stem;
	dy_array<dy_ot_record> records;
};
#endif


class dy_ot_history
{
public:
	dy_ot_history();

	dy_ot_record* create();
	void commit(dy_ot_record* rec);

	void undo();
	void redo();


//private:
	dy_ot_stamp m_stampserial;

	dy_ustack<dy_ot_record*> m_timeline;
	dy_ustack<dy_ot_record*> m_undone;

};


