#pragma once

typedef void dy_window;
int dy_engine_init();
void dy_engine_shutdown();
int dy_engine_living(dy_window* window);
void dy_engine_frame_begin();
void dy_engine_frame_end();
void dy_engine_event_pump();

void dy_engine_render(float f);

dy_window* dy_engine_new_window();
void dy_engine_window_select(dy_window* window);
void dy_engine_window_show(dy_window* window);
void* dy_engine_hwnd(dy_window* window);