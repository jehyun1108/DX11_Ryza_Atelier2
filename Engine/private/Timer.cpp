#include "Enginepch.h"
#include "Timer.h"
#include "TimeMgr.h"

// QueryPerformanceFrequency: �����ġ�� 1�ʿ� �� �� "°��"�Ÿ����� �˷��ش�. ������ ���ذ�.
// QueryPerformanceCounter: �����ġ�� �� ���� ��, ���ݱ��� �� �� �� "°��"�ŷȴ����� �˷��ִ� ���� ���̴�.

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