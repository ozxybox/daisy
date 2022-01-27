#include "engine.h"


int main()
{
	engine_init();
	while (engine_living())
		engine_frame_pump();
	engine_shutdown();
	return 0;
}