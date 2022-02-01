#include "dy_engine.h"


int main()
{
	dy_engine_init();
	dy_window* wnd = dy_engine_new_window();
	dy_engine_set_window(wnd);

	while (dy_engine_living(wnd))
	{
		dy_engine_event_pump();
		dy_engine_frame();
	}
	dy_engine_shutdown();
	return 0;
}