#include "dy_ustack.h"
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <XVirtualFileSystem.h>
#include <X9PServer.h>
#include <dy_math.h>
#include "launcher.h"

extern dy_camera s_camera;

static XVirtualFileSystem s_vfs;

static X9PServer s_api_sv;


void api_init()
{

	XVirtualFile* root = s_vfs.RootDirectory()->node;

	XVirtualFile* camf = new XVFSTypePointerFile<dy_camera>(&s_camera);
	root->AddChild(XSTR_L("camera"), camf, 0);

	s_api_sv.Begin(nullptr, "27016", &s_vfs);

	printf("Begin!\n");
}

void api_update()
{
	s_api_sv.AcceptConnections();
	s_api_sv.ProcessPackets();
}