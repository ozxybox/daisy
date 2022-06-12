#include "dy_ustack.h"
#include "X9PServer.h"
#include "XApiFileSystem.h"
#include <stdio.h>
#include <unordered_map>
#include "dy_brush.h"
#include "XIO.h"
#include "dy_editsys.h"



int main(int argc, const char** args)
{

	SystemInitSocket();

	dy_editsys_begin_server("27015");


	return 0;
}

#if 0
char s_filedata[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int s_filesize = sizeof(s_filedata);

struct hook_t
{
	X9PClient* cl;
	fid_t fid;
};
std::vector<hook_t> s_hooks;

void UpdateHooks(void* data, uint32_t count)
{

	if (s_hooks.size() > 0)
	{
		puts("I'm sending out an SOS to the world im sending out a");

		size_t sz = sizeof(Twrite_t) + count;
		Twrite_t* write = (Twrite_t*)calloc(sz, 1);

		write->size = sz;
		write->type = X9P_TWRITE;
		write->tag = NOTAG;
		write->offset = 0;
		write->count = count;
		memcpy(write->data(), data, count);

		for (hook_t& hook : s_hooks)
		{
			write->fid = hook.fid;
			hook.cl->QueueResponse(write);
		}

		free(write);
	}
}


class CTestFile : public XApiFSFile
{
public:
	virtual xerr_t Read(uint64_t offset, uint32_t count, void* buf, uint32_t& result) override
	{
		if (offset >= s_filesize) { result = 0; return 0; }
		count += offset;
		if (count > s_filesize)
			count = s_filesize;
		count -= offset;

		if(buf)
			memcpy(buf, &s_filedata[offset], count);
		result = count;
		return 0;
	}
	virtual xerr_t Write(uint64_t offset, uint32_t count, void* buf, uint32_t& result) override
	{
		if (offset >= s_filesize) { result = 0; return 0; }
		count += offset;
		if (count > s_filesize)
			count = s_filesize;
		count -= offset;

		if (buf)
		{
			memcpy(&s_filedata[offset], buf, count);
			UpdateHooks(&s_filedata[0], s_filesize);
		}
		result = count;
		return 0;
	}

	virtual uint64_t  Length() { return s_filesize; }

	virtual qidtype_t Type() { return X9P_QT_MOUNT; }
	virtual bool CanHook() { return true; }
	virtual xerr_t Hook()
	{
		XApiFSHandle* ndh = GetCaller();
		s_hooks.push_back({ ndh->hnd->auth->client, ndh->hnd->fid});

		return 0;
	}
};


struct dirtest
{
	xstr_t name;
	CTestFile file;
};

dirtest s_tree[] = {
	{XSTRL("one"),   {}},
	{XSTRL("two"),   {}},
	{XSTRL("three"), {}},
};

class CTestDir : public XApiFSDir
{
public:
	virtual xerr_t Create(xstr_t name, dirmode_t mode, XApiFSHandle** out) { return XERR_UNSUPPORTED; }
	virtual xerr_t Remove(xstr_t name) { return XERR_UNSUPPORTED; }
	virtual uint32_t ChildCount()
	{
		return sizeof(s_tree) / sizeof(dirtest);
	}
	virtual xerr_t FindChild(xstr_t name, XApiFSHandle* out)
	{
		for(int i = 0; i < sizeof(s_tree) / sizeof(dirtest); i++)
			if (xstrcmp(name, s_tree[i].name) == 0)
			{
				out->name = s_tree[i].name;
				out->node = &s_tree[i].file;
				out->parent = this;
				out->openmode = X9P_OPEN_EX_INVALID;
				return 0;
			}

		return XERR_FILE_DNE;
	}
	virtual xerr_t GetChild(uint32_t index, XApiFSHandle* out)
	{
		if (index >= sizeof(s_tree) / sizeof(dirtest))
			return XERR_FILE_DNE;

		out->name = s_tree[index].name;
		out->node = &s_tree[index].file;
		out->parent = this;
		out->openmode = X9P_OPEN_EX_INVALID;
		return 0;
	}
};


int main(int argc, const char** args)
{

	SystemInitSocket();
	if (argc == 1)
	{
		CTestDir rootdir;
		XApiFileSystem vfs(&rootdir);



		X9PServer sv;
		sv.Begin(nullptr, "27015", &vfs);

		printf("Begin!\n");


		while (1)
		{
			sv.AcceptConnections();
			sv.ProcessPackets();
		}
	}
	else
	{
		X9PClient cl;
		cl.Begin("127.0.0.1", "27015");
		
		XAuth auth = {&cl, XSTRL("username"), 0};

		XFile f;
		f.Attach(&auth, XSTRL("/"), &cl);

		XFile hook;
		f.Walk(hook, "one");
		f.Await();


		hook.Hook(
			[&](xerr_t err, uint64_t offset, uint32_t count, void* data) {
				puts("new data on the hook");
				fwrite(data, 1, count, stdout);
			},
			
			[&](xerr_t err, qid_t* qid, uint32_t iounit) {
				if (err)
					puts(err);
				else
					puts("yay");
			});

		hook.Await();

		char hello[] = "Hello there!";
		hook.Write(0, sizeof(hello), &hello[0]);
		while (1)
		{
			cl.ProcessPackets();
		}
	}
	return 0;
}

#endif