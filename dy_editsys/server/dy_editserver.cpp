#include "csg/dy_csg.h"
#include "csg/dy_brush.h"
#include "dy_ustack.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <XApiFileSystem.h>
#include <socket/TCPSocket.h>
#include <X9PServer.h>
#include "dy_snooze.h"
#include "dy_netdb.h"
#include "dy_ot.h"
#include "dy_editsys.h"

extern dy_netdb_tree s_tree;


struct dy_edituser
{
	XAuth* auth;
	dy_edituser_info info;
	dy_ot_history journal;
	dy_ot_record* currentrecord = 0;
};
static dy_ustack<dy_edituser> s_users;
dy_ot_history s_worldhistory;


// FIXME: Slow lookup time!
dy_edituser* GetUserFromID(int id)
{
	for (auto* u : s_users)
	{
		if (u->auth->id == id)
			return u;
	}
	return 0;
}

class XApiFSList : public XApiFSDir
{
public:

	virtual XApiFSNode* GetNode(uint32_t i) = 0;
	virtual XApiFSHandle GetNodeHandle(uint32_t i) = 0;

	virtual xerr_t FindChild(xstr_t name, XApiFSHandle* out)
	{
		char buf[255];
		xstr_t fname = (xstr_t)&buf[0];

		for (uint32_t i = 0; i < ChildCount(); i++)
		{
			XApiFSNode* node = GetNode(i);
			fsapisetcaller sc(GetCaller(), node); // FIXME: Terrible! The child could be using the qid path as its identifier and get confused!

			node->Name(fname, 255);
			if (xstrcmp(name, fname) == 0)
			{
				// Found it!
				if(out)
					*out = GetNodeHandle(i);
				return 0;
			}
		}

		return XERR_FILE_DNE;
	}
	virtual xerr_t GetChild(uint32_t index, XApiFSHandle* out)
	{
		if (index >= ChildCount())
			return XERR_IOOR;

		if (out)
			*out = GetNodeHandle(index);

		return 0;
	}


};



class CEditServerFileSystem : public XApiFileSystem
{
public:
	CEditServerFileSystem(XApiFSDir* root) : XApiFileSystem(root) {}

//	virtual void Disconnect(XAuth* auth) {};

	virtual void NewConnection(XAuth* auth)
	{
		printf("New user ");
		xstrprint(auth->uname);
		putchar('\n');

		s_users.push({ auth, {}, {}, 0 });
	};
};


// World file system

class CApiPlaneFile : public XApiFSFile
{
public:

	dy_bplane* ThisPlane()
	{
		XApiFSHandle* caller = GetCaller();
		return static_cast<dy_bplane*>(s_tree.find(caller->qidpath)->ptr());
	}

	virtual uint64_t Length() override { return sizeof(dy_bplane); }

	virtual size_t Name(xstr_t out, size_t avail) override
	{
		XApiFSHandle* caller = GetCaller();
		return xstrnfromd(caller->qidpath, 16, out, avail);
	}
	
	virtual xerr_t Read(uint64_t offset, uint32_t count, void* buf, uint32_t& result) override
	{
		if (offset != 0 || count < sizeof(dy_bplane))
		{
			result = 0;
			return 0;
		}
		result = sizeof(dy_bplane);

		if (buf)
		{
			dy_bplane* plane = ThisPlane();
			if (!plane)
				return "LOL";

			memcpy(buf, plane, sizeof(dy_bplane));
		}

		return 0;
	}
	
	virtual xerr_t Write(uint64_t offset, uint32_t count, void* buf, uint32_t& result) override
	{
		return XERR_UNSUPPORTED;
	}

};
static CApiPlaneFile s_fileplane;


class CApiSolidDir : public XApiFSDir
{
public:

	virtual size_t Name(xstr_t out, size_t avail) override
	{
		XApiFSHandle* caller = GetCaller();
		return xstrnfromd(caller->qidpath, 16, out, avail);
	}

	virtual uint32_t ChildCount()
	{
		XApiFSHandle* caller = GetCaller();
		const dy_array<dy_netdb_obj*> children = s_tree.children(caller->qidpath);
		
		return children.count;
	}

	virtual xerr_t FindChild(xstr_t name, XApiFSHandle* out)
	{
		XApiFSHandle* caller = GetCaller();
		const dy_array<dy_netdb_obj*> children = s_tree.children(caller->qidpath);
		if (!children.data)
			return "Solid does not exist";

		int d = 0;
		if (!xstrtod(name, 16, &d) || d < 0) 
			return "Invalid name";
		
		// FIXME: Use a lookup!
		for (int i = 0; i < children.count; i++)
		{
			if (children[i]->id() == d)
			{
				*out = { 0, X9P_OPEN_EX_INVALID, (uint64_t)d, &s_fileplane, this };
				return 0;
			}
		}

		return "Plane does not exist";
	}
	virtual xerr_t GetChild(uint32_t index, XApiFSHandle* out)
	{
		XApiFSHandle* caller = GetCaller();
		const dy_array<dy_netdb_obj*> children = s_tree.children(caller->qidpath);
		if (!children.data)
			return "Solid does not exist";

		if (index >= children.count)
			return XERR_IOOR;
		
		if(out)
			*out = { 0, X9P_OPEN_EX_INVALID, (uint64_t)children[index]->id(), &s_fileplane, this};

		return 0;
	}


	virtual xerr_t Create(xstr_t name, dirmode_t mode, XApiFSHandle* out)
	{
#if 1
		return XERR_CANT_CREATE;
#else
		XApiFSHandle* caller = GetCaller();
		if (s_tree.find(caller->qidpath) == 0)
			return XERR_CANT_CREATE;

		dy_netdb_obj* solid = s_tree.create(caller->qidpath, DY_NETDB_TYPE_PLANE, new dy_bplane);

		if (out)
			*out = { 0, X9P_OPEN_EX_INVALID, (uint64_t)solid->id(), &s_fileplane, this };

		return 0;
#endif
	}

};
static CApiSolidDir s_dirsolid;



class CApiWorldDir : public XApiFSDir
{
public:

	virtual size_t Name(xstr_t out, size_t avail)
	{
		const xstr_t name = XSTRL("solids");
		if (out && avail >= name->len)
			memcpy(out, name, name->size());
		return name->size();
	}

	virtual uint32_t ChildCount() { return s_tree.objectsForType(DY_NETDB_TYPE_SOLID).count; }

	virtual xerr_t FindChild(xstr_t name, XApiFSHandle* out)
	{
		int d = 0;
		if (!xstrtod(name, 16, &d) || d < 0)
			return "Invalid name";

		dy_ustack<dy_netdb_obj*>& solids = s_tree.objectsForType(DY_NETDB_TYPE_SOLID);

		// FIXME: Use a look up table instead of this!
		for(dy_netdb_obj** s : solids)
		{
			if ((*s)->id() == d)
			{
				*out = { 0, X9P_OPEN_EX_INVALID, (uint64_t)d, &s_dirsolid, this };
				return 0;
			}

		}

		return "Solid does not exist";
	}
	virtual xerr_t GetChild(uint32_t index, XApiFSHandle* out)
	{
		dy_ustack<dy_netdb_obj*>& solids = s_tree.objectsForType(DY_NETDB_TYPE_SOLID);

		if (index >= solids.count)
			return XERR_IOOR;

		if (out)
			*out = { 0, X9P_OPEN_EX_INVALID, (uint64_t)(*solids.seek(index))->id(), &s_dirsolid, this};

		return 0;
	}


	virtual xerr_t Create(xstr_t name, dirmode_t mode, XApiFSHandle* out)
	{
#if 1
		return XERR_CANT_CREATE;
#else
		dy_netdb_obj* solid = s_tree.create(0, DY_NETDB_TYPE_SOLID, new dy_bsolid);

		if(out)
			*out = { 0, X9P_OPEN_EX_INVALID, (uint64_t)solid->id(), &s_dirsolid, this};

		return 0;
#endif
	}

};
static CApiWorldDir s_dirworld;


class CHookTest
{
public:

	void AddHook(XApiFSHandle* caller)
	{
		if (m_hooklist.find(caller->qidpath) == m_hooklist.end())
			m_hooklist[caller->qidpath] = {};

		m_hooklist[caller->qidpath].push_back(*caller->hnd);
	}

	void RemoveHook(XApiFSHandle* caller)
	{
		if (m_hooklist.find(caller->qidpath) == m_hooklist.end())
			m_hooklist[caller->qidpath] = {};

		m_hooklist[caller->qidpath].push_back(*caller->hnd);
	}
	
	void Broadcast()
	{

	}

	std::unordered_map<int, std::vector<xhnd_data>> m_hooklist;
};



CHookTest s_infohooklist;
// User info file system
class CApiUserInfoFile : public XApiFSFile
{
public:
	
	dy_edituser* ThisUser()
	{
		int id = GetCaller()->qidpath;
		return GetUserFromID(id);
	}

	virtual size_t Name(xstr_t out, size_t avail)
	{
		const xstr_t name = XSTRL("info");
		if (out && avail >= name->len)
			memcpy(out, name, name->size());
		return name->size();
	}


	virtual uint64_t Length() { return sizeof(dy_edituser_info); }

	virtual xerr_t Read(uint64_t offset, uint32_t count, void* buf, uint32_t& result) override
	{
		if (offset != 0 || count < sizeof(dy_edituser_info))
		{
			result = 0;
			return 0;
		}
		result = sizeof(dy_edituser_info);

		if (buf)
		{
			dy_edituser* user = ThisUser();
			memcpy(buf, &user->info, sizeof(dy_edituser_info));
		}

		return 0;
	}
	virtual xerr_t Write(uint64_t offset, uint32_t count, void* buf, uint32_t& result) override
	{
		if (!CanHook())
			return XERR_NO_PERM;

		if (offset != 0 || count < sizeof(dy_edituser_info))
		{
			result = 0;
			return XERR_PARTIAL_RW;
		}
		result = sizeof(dy_edituser_info);

#if 0 
		if (buf)
		{
			dy_edituser* user = ThisUser();
			memcpy(&user->info, buf, sizeof(dy_edituser_info));


			auto f = s_hooklist.find(GetCaller()->qidpath);
			if (f != s_hooklist.end())
			{

				std::vector<xhnd_data>& hooked = f->second;

				const int sz = sizeof(Twrite_t) + sizeof(dy_edituser_info);
				char buf[sz];

				Twrite_t* w = (Twrite_t*)&buf[0];
				w->size = sz;
				w->type = X9P_TWRITE;
				w->tag = NOTAG;

				w->offset = 0;
				w->count = sizeof(dy_edituser_info);
				memcpy(w->data(), &user->info, sizeof(dy_edituser_info));

				for (xhnd_data& d : hooked)
				{
					w->fid = d.fid;
					d.auth->client->QueueResponse(w);
				}

			}
		}
#endif 

		return 0;
	}



	virtual bool CanHook()
	{
		XApiFSHandle* caller = GetCaller();
		return GetUserFromID(caller->qidpath)->auth != caller->hnd->auth;
	}
	virtual xerr_t Hook()
	{
		XApiFSHandle* caller = GetCaller();

		s_infohooklist.AddHook(caller);
		return 0;
	}
	virtual void Clunk()
	{
		XApiFSHandle* caller = GetCaller();
		s_infohooklist.RemoveHook(caller);

	}

};
static CApiUserInfoFile s_fileuserinfo;



static XApiFSNode* s_filetreeuser[] = {
	&s_fileuserinfo,
};
class CApiUserDir : public XApiFSList
{
public:

	dy_edituser* ThisUser()
	{
		int id = GetCaller()->qidpath;
		return GetUserFromID(id);
	}

	virtual size_t Name(xstr_t out, size_t avail)
	{
		dy_edituser* user = ThisUser();
		xstr_t name = user ? user->auth->uname : XSTRL("|");
		if (out && avail >= name->len)
			memcpy(out, name, name->size());
		return name->size();
	}


	virtual uint32_t ChildCount() { return sizeof(s_filetreeuser) / sizeof(XApiFSNode*); }
	virtual XApiFSNode* GetNode(uint32_t i) { return s_filetreeuser[i]; }
	virtual XApiFSHandle GetNodeHandle(uint32_t i) { return { 0, X9P_OPEN_EX_INVALID, GetCaller()->qidpath, s_filetreeuser[i], this}; }

};
static CApiUserDir s_diruser;


class CApiUserListDir : public XApiFSDir
{
public:

	virtual size_t Name(xstr_t out, size_t avail)
	{
		const xstr_t name = XSTRL("users");
		if (out && avail >= name->len)
			memcpy(out, name, name->size());
		return name->size();
	}

	virtual uint32_t ChildCount() { return s_users.count; }

	virtual xerr_t FindChild(xstr_t name, XApiFSHandle* out)
	{
		for (auto* u : s_users)
		{
			if (xstrcmp(u->auth->uname, name) == 0)
			{
				if(out)
					*out = { 0, X9P_OPEN_EX_INVALID, (uint64_t)u->auth->id, &s_diruser, this };
				return 0;
			}
		}
		return XERR_FILE_DNE;
	}
	virtual xerr_t GetChild(uint32_t index, XApiFSHandle* out)
	{
		if (index >= ChildCount())
			return XERR_IOOR;

		if (out)
			*out = { 0, X9P_OPEN_EX_INVALID, (uint64_t)s_users.seek(index)->auth->id, &s_diruser, this };

		return 0;
	}

};
CApiUserListDir s_diruserlist;


// Network objects
class CApiNetworkObjectFile : public XApiFSFile
{
public:

	dy_bplane* ThisPlane()
	{
		XApiFSHandle* caller = GetCaller();
		return static_cast<dy_bplane*>(s_tree.find(caller->qidpath)->ptr());
	}

	virtual uint64_t Length() override { return sizeof(dy_bplane); }

	virtual size_t Name(xstr_t out, size_t avail) override
	{
		XApiFSHandle* caller = GetCaller();
		return xstrnfromd(caller->qidpath, 16, out, avail);
	}
	
	virtual xerr_t Read(uint64_t offset, uint32_t count, void* buf, uint32_t& result) override
	{
	//	if (offset != 0 || count < sizeof(dy_bplane))
		//{
			result = 0;
			return 0;
		//}
		result = sizeof(dy_bplane);

		if (buf)
		{
			dy_bplane* plane = ThisPlane();
			if (!plane)
				return "LOL";

			memcpy(buf, plane, sizeof(dy_bplane));
		}

		return 0;
	}
	
	virtual xerr_t Write(uint64_t offset, uint32_t count, void* buf, uint32_t& result) override
	{
		return XERR_UNSUPPORTED;
	}

};
static CApiNetworkObjectFile s_filenetobj;

class CApiObjectsDir : public XApiFSDir
{
public:

	virtual size_t Name(xstr_t out, size_t avail)
	{
		const xstr_t name = XSTRL("objects");
		if (out && avail >= name->len)
			memcpy(out, name, name->size());
		return name->size();
	}

	virtual uint32_t ChildCount() { return s_tree.objectCount(); }

	virtual xerr_t FindChild(xstr_t name, XApiFSHandle* out)
	{
		int d = 0;
		if (!xstrtod(name, 16, &d) || d < 0)
			return "Invalid name";


		dy_netdb_obj* o = s_tree.find(d);
		if (o)
		{
			*out = { 0, X9P_OPEN_EX_INVALID, (uint64_t)d, &s_dirsolid, this };
			return 0;
		}


		return "Solid does not exist";
	}
	virtual xerr_t GetChild(uint32_t index, XApiFSHandle* out)
	{
		if (index >= s_tree.objectCount())
			return XERR_IOOR;

		if (out)
			*out = { 0, X9P_OPEN_EX_INVALID, (uint64_t)s_tree.objectAtIndex(index)->id(), &s_filenetobj, this};

		return 0;
	}


	virtual xerr_t Create(xstr_t name, dirmode_t mode, XApiFSHandle* out) { return XERR_CANT_CREATE; }

};
CApiObjectsDir s_dirobjects;

// OT Operator
class CApiOTRecordComplete : public XApiFSFile
{
public:
	virtual size_t Name(xstr_t out, size_t avail)
	{
		int id = GetCaller()->qidpath;
		return xstrnfromd(id, 16, out, avail);
	}

	virtual uint64_t Length() { return 0; }

	virtual xerr_t Read(uint64_t offset, uint32_t count, void* buf, uint32_t& result) override
	{
		return 0;
	}
	virtual xerr_t Write(uint64_t offset, uint32_t count, void* buf, uint32_t& result) override
	{
		return 0;
	}
};
static CApiOTRecordComplete s_fileotcomplete;


class CApiOTRecordInProgress : public XApiFSFile
{
public:

	virtual size_t Name(xstr_t out, size_t avail)
	{
		int id = GetCaller()->qidpath;
		return xstrnfromd(id, 16, out, avail);
	}

	virtual uint64_t Length() { return 0; }

	virtual xerr_t Read(uint64_t offset, uint32_t count, void* buf, uint32_t& result) override
	{
		return 0;
	}
	virtual xerr_t Write(uint64_t offset, uint32_t count, void* buf, uint32_t& result) override
	{
		dy_edituser* user = GetUserFromID(GetCaller()->hnd->auth->id);
		dy_ot_record* rec = user->currentrecord;
		if (!rec || rec->stamp != GetCaller()->qidpath)
			return XERR_FILE_DNE;

		size_t opsize = rec->op->size();

		// We can only write exactly the op's data
		if (count != opsize || offset != 0)
		{
			result = 0;
			return XERR_PARTIAL_RW;
		}
		result = opsize;

		// Pass it into the OP
		if (buf)
			rec->op->apply(buf);

		return 0;
	}

	virtual void Clunk() override
	{
		// Commit the record
		dy_edituser* user = GetUserFromID(GetCaller()->hnd->auth->id);
		if (!user->currentrecord || user->currentrecord->stamp != GetCaller()->qidpath)
			return;

		s_worldhistory.commit(user->currentrecord);
		user->currentrecord = 0;
	}
};
static CApiOTRecordInProgress s_fileotinprogress;


// OT Timeline
class CApiOTTimeline : public XApiFSDir
{
public:
	
	virtual size_t Name(xstr_t out, size_t avail)
	{
		const xstr_t name = XSTRL("timeline");
		if (out && avail >= name->len)
			memcpy(out, name, name->size());
		return name->size();
	}

	virtual uint32_t ChildCount()
	{
		// FIXME: GROSS!!!!
		int i = GetUserFromID(GetCaller()->hnd->auth->id)->currentrecord == 0 ? 0 : 1;
		return s_worldhistory.m_timeline.count + i;
	}

	virtual xerr_t FindChild(xstr_t name, XApiFSHandle* out)
	{
		int d = 0;
		if (!xstrtod(name, 16, &d) || d < 0)
			return "Invalid name";

		dy_edituser* user = GetUserFromID(GetCaller()->hnd->auth->id);
		if (user->currentrecord && d == user->currentrecord->stamp)
		{
			// Found it, it's the inprogress record!
			*out = { 0, X9P_OPEN_EX_INVALID, (uint64_t)d, &s_fileotinprogress, this };
			return 0;
		}

		// FIXME: Use a look up table instead of this!
		for(dy_ot_record** prec : s_worldhistory.m_timeline)
		{
			if ((*prec)->stamp == d)
			{
				*out = { 0, X9P_OPEN_EX_INVALID, (uint64_t)d, &s_fileotcomplete, this };
				return 0;
			}

		}

		return "Solid does not exist";
	}
	virtual xerr_t GetChild(uint32_t index, XApiFSHandle* out)
	{
		dy_edituser* user = GetUserFromID(GetCaller()->hnd->auth->id);
		
		if(index > s_worldhistory.m_timeline.count)
			return XERR_IOOR;

		if (user->currentrecord)
		{
			// Last one is always the in progress rec
			if (index == s_worldhistory.m_timeline.count)
			{
				*out = { 0, X9P_OPEN_EX_INVALID, (uint64_t)user->currentrecord->stamp, &s_fileotinprogress, this };
				return 0;
			}
		}
		else
		{
			// We don't have a record in progress at the moment
			if (index == s_worldhistory.m_timeline.count)
				return XERR_IOOR;
		}


		if (out)
			*out = { 0, X9P_OPEN_EX_INVALID, (uint64_t)(*s_worldhistory.m_timeline.seek(index))->stamp, &s_fileotcomplete, this};

		return 0;
	}


	virtual xerr_t Create(xstr_t name, dirmode_t mode, XApiFSHandle* out)
	{
		dy_ot_operator* op = dy_ot_create_operator(name);
		if (!op)
			return "Invalid operator classname";

		dy_edituser* user = GetUserFromID(GetCaller()->hnd->auth->id);

		// If we already have a record in progress, use it
		if (user->currentrecord)
		{
			delete user->currentrecord->op;
			user->currentrecord->op = op;
			*out = { 0, X9P_OPEN_EX_INVALID, (uint64_t)user->currentrecord->stamp, &s_fileotinprogress, this };
			return 0;
		}

		// Create a new record for this user
		dy_ot_record* rec = s_worldhistory.create();
		rec->op = op;
		user->currentrecord = rec;

		*out = { 0, X9P_OPEN_EX_INVALID, (uint64_t)rec->stamp, &s_fileotinprogress, this};

		return 0;
	}

};
static CApiOTTimeline s_dirottimeline;



// Root of the file system
static XApiFSNode* s_rootfiletree[] = {
	&s_dirworld,
	&s_diruserlist,
	&s_dirobjects,
	&s_dirottimeline,
};
class CApiRootDir : public XApiFSList
{
public:

	virtual size_t Name(xstr_t out, size_t avail)
	{
		const xstr_t name = XSTRL("/");
		if (out && avail >= name->len)
			memcpy(out, name, name->size());
		return name->size();
	}


	virtual uint32_t ChildCount()
	{
		return sizeof(s_rootfiletree) / sizeof(XApiFSNode*);
	}
	virtual XApiFSNode* GetNode(uint32_t i) { return s_rootfiletree[i]; }
	virtual XApiFSHandle GetNodeHandle(uint32_t i) { return { 0, X9P_OPEN_EX_INVALID, 0, s_rootfiletree[i], this }; }

};
static CApiRootDir s_dirroot;



void dy_editsys_begin_server(const char* port)
{

#if 0
	dy_bplane* planes = new dy_bplane[] {
		{{ 1, 0, 0},64},
		{{ 0, 1, 0},64},
		{{ 0, 0, 1},64},
		{{-1, 0, 0},64},
		{{ 0,-1, 0},64},
		{{ 0, 0,-1},64},
	};

	dy_netdb_obj* sobj = s_tree.create(0, DY_NETDB_TYPE_SOLID, 0);
	for(int i = 0; i < 6; i++)
		s_tree.create(sobj->id(), DY_NETDB_TYPE_PLANE, planes + i);
#endif


	CEditServerFileSystem vfs(&s_dirroot);



	X9PServer sv;
	sv.Begin(nullptr, "27015", &vfs);

	printf("Begin!\n");


	float acceptDelay = 1.0 / 2.0;
	float acctime = 0;

	CSnoozeTimer snoozer(3.0, 4.0);


	while (1)
	{
		snoozer.BeginFrame();
		float curtime = snoozer.Time();
		if (curtime >= acctime)
		{
			sv.AcceptConnections();
			acctime = curtime + acceptDelay;
		}

		sv.ProcessPackets();

		// Sleep a bit between processing... Zzzz... Zzzz...
		//snoozer.EndFrame(sv.s_lastRecvTime);
	}
}

