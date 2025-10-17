#include "Enginepch.h"
#include "Timer.h"
#include "TimeMgr.h"

// QueryPerformanceFrequency: 스톱워치가 1초에 몇 번 "째깍"거리는지 알려준다. 고정된 기준값.
// QueryPerformanceCounter: 스톱워치를 딱 봤을 때, 지금까지 총 몇 번 "째깍"거렸는지를 알려주는 현재 값이다.

unique_ptr<Timer> Timer::Create()
{
	auto instance = make_unique<Timer>();

	if (FAILED(instance->Ready_Timer()))
		return nullptr;

	return instance;
}

HRESULT Timer::Ready_Timer()
{
	QueryPerformanceCounter(&frameTime);		
	QueryPerformanceCounter(&lastTime);		
	QueryPerformanceCounter(&fixTime);		

	QueryPerformanceFrequency(&cpuTick);

	return S_OK;
}

void Timer::Tick()
{
	QueryPerformanceCounter(&frameTime);

	if (frameTime.QuadPart - fixTime.QuadPart >= cpuTick.QuadPart)
	{
		QueryPerformanceFrequency(&cpuTick);
		fixTime = frameTime;
	}

	dt = (frameTime.QuadPart - lastTime.QuadPart) / static_cast<float>(cpuTick.QuadPart);

	lastTime = frameTime;
}