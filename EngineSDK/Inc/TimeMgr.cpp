#include "Enginepch.h"
#include "TimeMgr.h"
#include "Timer.h"

unique_ptr<TimeMgr> TimeMgr::Create()
{
	auto instance = make_unique<TimeMgr>();

	if (FAILED(instance->Init()))
		return nullptr;

	return instance;
}

HRESULT TimeMgr::Init()
{
	for (size_t i{}; i < timers.size(); ++i)
	{
		timers[i] = Timer::Create();
		if (!timers[i])
			return E_FAIL;
	}
	return S_OK;
}

_float TimeMgr::GetDt(TIMER timerID)
{
	auto timer = FindTimer(timerID);
	return timer->GetDt();
}

void TimeMgr::UpdateDt(TIMER timerID)
{
	auto timer = FindTimer(timerID);
	timer->Tick();
}

Timer* TimeMgr::FindTimer(TIMER timerID)
{
	const size_t idx = static_cast<size_t>(timerID);
	
	if (idx >= timers.size() || !timers[idx])
		return nullptr;

	return timers[idx].get();
}