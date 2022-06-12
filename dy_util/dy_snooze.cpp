#include "dy_snooze.h"
#include <thread>
#include <chrono>

CSnoozeTimer::CSnoozeTimer(double drowseStartTime, double drowseUPS, double dozeStartTime, double dozeUPS)
	: m_drowseStartTime(drowseStartTime), m_drowseFrameTime(1.0 / drowseUPS), m_dozeStartTime(dozeStartTime), m_dozeFrameTime(1.0 / dozeUPS)
{
	m_creation = std::chrono::high_resolution_clock::now();
	m_curtime = m_creation;
}

void CSnoozeTimer::BeginFrame()
{
	m_curtime = std::chrono::high_resolution_clock::now();
}

void CSnoozeTimer::EndFrame(double lastUpdate)
{
	auto lu = std::chrono::duration<double>(lastUpdate) + m_drowseStartTime;
	auto t = m_curtime - m_creation;

	// Zzzz... Zzzz...
	if (t < lu)
		return;

	// As we fall deeper into our sleep, lower our fps further
	auto snoozePercent = (t - lu) / (m_dozeStartTime - m_drowseStartTime);
	if (snoozePercent > 1.0)
		snoozePercent = 1.0;
	auto frameTime = m_drowseFrameTime * (1.0 - snoozePercent) + m_dozeFrameTime * snoozePercent;


	auto now = std::chrono::high_resolution_clock::now();
	auto dt = now - m_curtime;

	// If we're past our delay's timeframe, drop out now
	if (dt >= frameTime)
		return;

	auto snoreDuration = frameTime - dt;
	std::this_thread::sleep_for(snoreDuration);
}

double CSnoozeTimer::Time()
{
	return std::chrono::duration_cast<std::chrono::duration<double>>(m_curtime - m_creation).count();
}

