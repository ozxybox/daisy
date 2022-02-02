#include "dy_engine.h"


int main()
{
	dy_engine_init();
	dy_window* wnd = dy_engine_new_window();
	dy_engine_set_window(wnd);

	while (dy_engine_living(wnd))
	{
		dy_engine_event_pump();
		dy_engine_frame_begin();
		dy_engine_render(0.0f);
		dy_engine_frame_end();
	}
	dy_engine_shutdown();
	return 0;
}