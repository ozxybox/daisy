#include "X9PServer.h"
#include "socket/TCPSocket.h"
#include "vpk_fs.h"
#include "xstring.h"
#include <iostream>
#include <assert.h>


int main(int argc, const char** args)
{
	if (argc != 2)
	{
		puts("Expected use:\n\tdy_plugin_vpk.exe \"path/to/pak_dir.vpk\"");
		return 1;
	}

	const char* path = args[1];

	SystemInitSocket();


	CVPKFileSystem vpkfs(path);


	// Server Mode
	puts("Hosting server on localhost:27015\n");

	X9PServer sv;
	sv.Begin(nullptr, "27015", &vpkfs);

	float curtime = 0;
	float last_acc = 0;
	while (1)
	{
		//printf(".");
		//std::this_thread::sleep_for(std::chrono::milliseconds(2));


		curtime = clock() / (float)CLOCKS_PER_SEC;
		if (curtime > last_acc + 1.5)
		{
			sv.AcceptConnections();
			last_acc = curtime;
		}

		sv.ProcessPackets();
	}

	sv.End();

	return 0;
}
