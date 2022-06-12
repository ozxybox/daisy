#pragma once
#include <chrono>

// Drowse: Feeling sleepy
// Doze:   A light sleep
class CSnoozeTimer
{
public:

	CSnoozeTimer(double drowseStartTime, double drowseUPS, double dozeStartTime, double dozeUPS);
	CSnoozeTimer(double dozeStartTime, double dozeUPS) : CSnoozeTimer(dozeStartTime, dozeUPS, dozeStartTime, dozeUPS) {}
	void BeginFrame();
	void EndFrame(double lastUpdate);
	double Time();

private:

	const std::chrono::duration<double> m_drowseStartTime;
	const std::chrono::duration<double> m_drowseFrameTime;

	const std::chrono::duration<double> m_dozeStartTime;
	const std::chrono::duration<double> m_dozeFrameTime;


	std::chrono::high_resolution_clock::time_point m_creation;
	std::chrono::high_resolution_clock::time_point m_curtime;
};
