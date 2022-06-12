#include "engine/dy_engine.h"
#include "dy_editor.h"
#include <time.h>
#include <stdio.h>
#include <thread>
#include <chrono>
#include "dy_snooze.h"

float inputSnoozeCheck = 3.0;
float inputSnoozeTime = 1.0 / 22.0;

float inputSnoreCheck = 33.0;
float inputSnoreTime = 1.0 / 14.0;

int main_poser();

extern dy_camera s_camera;
int main(int argc, const char** args)
{

	dy_engine_init();
	dy_editor_init(argc, args);

//	if (1)
//		return main_poser();


	// Create the main window
	dy_window* wnd = dy_engine_new_window();
	dy_engine_window_show(wnd);
	dy_engine_window_select(wnd);

	float targetFps = 60.0;
	float targetFrameTime = 1.0 / targetFps;

	float lastframetime = dy_engine_update_time();
	
	bool begin_camera = false;
	double lmx = 0, lmy = 0;

	float inputLastTime = 0;
	
	CSnoozeTimer snoozer(0,0);

	// Main loop
	while (dy_engine_living(wnd))
	{
		float curtime = dy_engine_update_time();
		float dt = curtime - lastframetime;
		lastframetime = curtime;


		dy_engine_window_select(wnd);
		dy_engine_event_pump();
		dy_engine_frame_begin();
		


		if (dy_engine_key_down(32))
		{
			double mx = 0, my = 0;
			dy_engine_mouse_pos(&mx, &my);
			if (!begin_camera)
			{
				lmx = mx;
				lmy = my;
			}

			double dmx = mx - lmx;
			double dmy = my - lmy;
			lmx = mx;
			lmy = my;

			s_camera.rotation.pitch += dmy / 640.0f * (DY_PI * 2.0);
			s_camera.rotation.yaw   += dmx / 240.0f * (DY_PI * 0.5);

			if (s_camera.rotation.pitch >  DY_PI * 0.5) s_camera.rotation.pitch =  DY_PI * 0.5;
			if (s_camera.rotation.pitch < -DY_PI * 0.5) s_camera.rotation.pitch = -DY_PI * 0.5;

			begin_camera = true;
		}
		else
			begin_camera = false;

		float movespeed = 1024;
		vec4 mv = { 0,0,0,1 };
		if (dy_engine_key_down(87)) // W
			mv.z -= 1;
		if (dy_engine_key_down(83)) // S
			mv.z += 1;
		if (dy_engine_key_down(65)) // A
			mv.x -= 1;
		if (dy_engine_key_down(68)) // D
			mv.x += 1;

		// TODO: Totally slow!
		// Rotate the movement towards our camera
		mv = mat4::yrotation(s_camera.rotation.yaw) * mat4::xrotation(s_camera.rotation.pitch) * mv;

		if (dy_engine_key_down(69)) // E
			mv.y -= 1;
		if (dy_engine_key_down(81)) // Q
			mv.y += 1;

		if (dy_engine_key_down(340)) // Left Shift
			movespeed *= 0.125f;

		if (mv.x != 0 || mv.y != 0 || mv.z != 0 || begin_camera)
		{
			mv = mv.normalized() * movespeed * dt;
			s_camera.origin += vec3{ mv.x, mv.y, mv.z };

			inputLastTime = curtime;
			dy_editor_update_camera();
		}
		

		dy_editor_update();
		dy_editor_render();
		dy_engine_frame_end();


		// FPS Control
		if (curtime > inputLastTime + inputSnoozeCheck)
		{
			// As we fall deeper into our sleep, lower our fps further
			float snore = (curtime - (inputLastTime + inputSnoozeCheck)) / (inputSnoreCheck - inputSnoozeCheck);
			if (snore > 1.0)
				snore = 1.0;
			float frameTime = inputSnoozeTime * (1.0 - snore) + inputSnoreTime * snore;

			// If there has been no input for more than "inputSnoozeCheck" seconds, start sleeping to control our frame time
			float nextFrame = curtime + frameTime;
			int sleepTimeMS = nextFrame - dy_realtime();

			std::this_thread::sleep_for(std::chrono::duration<double>(sleepTimeMS));
		}

	}
	dy_engine_shutdown();
	return 0;
}


#if 0
int main_poser()
{

	// Create the main window
	dy_window* wnd = dy_engine_new_window();
	dy_engine_window_show(wnd);
	dy_engine_window_select(wnd);


	float lastframetime = dy_engine_update_time();

	bool begin_camera = false;
	double lmx = 0, lmy = 0;

	float zoom = 2048;
	s_camera.origin = dy_vec_forward(s_camera.rotation) * -zoom;


	// Main loop
	while (dy_engine_living(wnd))
	{
		float curtime = dy_engine_update_time();
		float dt = curtime - lastframetime;
		lastframetime = curtime;


		dy_engine_window_select(wnd);
		dy_engine_event_pump();
		dy_engine_frame_begin();


		double mx = 0, my = 0;
		dy_engine_mouse_pos(&mx, &my);

		double dmx = mx - lmx;
		double dmy = my - lmy;

		if (dy_engine_mouse_down(0))
		{
			s_camera.rotation.pitch += dmy / 640.0f * (DY_PI * 2.0);
			s_camera.rotation.yaw   += dmx / 240.0f * (DY_PI * 0.5);

			s_camera.origin = dy_vec_forward(s_camera.rotation) * -zoom;
		}

		if (dy_engine_mouse_down(1))
		{
			float dz = dmy * (zoom / 20.0f);

			// Don't allow zooming into and past our model
			zoom += dz;
			if (zoom <= 1)
				zoom = 1;

			s_camera.origin = dy_vec_forward(s_camera.rotation) * -zoom;

		}

		dy_editor_update_camera();

		lmx = mx;
		lmy = my;

		dy_editor_update();
		dy_editor_render();
		dy_engine_frame_end();


	}
	dy_engine_shutdown();
	return 0;
}
#endif