#pragma once
#include "dy_vertex.h"

// Updates the result of dy_curtime. Returns the new time
float dy_engine_update_time();
// Return the time of the current frame
float dy_curtime();
float dy_realtime();

typedef void dy_window;
int dy_engine_init();
void dy_engine_shutdown();
int dy_engine_living(dy_window* window);
void dy_engine_frame_begin();
void dy_engine_frame_end();
void dy_engine_event_pump();


dy_window* dy_engine_new_window();
void dy_engine_window_size(int* w, int* h); // dimensions of the current selected window
void dy_engine_window_select(dy_window* window);
void dy_engine_window_show(dy_window* window);
void* dy_engine_hwnd(dy_window* window);

int dy_engine_key_down(int key);
int dy_engine_mouse_down(int button); 
void dy_engine_mouse_pos(double* mx, double* my);