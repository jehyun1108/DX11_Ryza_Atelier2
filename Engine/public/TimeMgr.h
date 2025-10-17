#pragma once

#include "Timer.h"

NS_BEGIN(Engine)

class TimeMgr final
{
public:
	static unique_ptr<TimeMgr> Create();

	HRESULT Init();
	_float GetDt(TIMER timerID);
	void UpdateDt(TIMER timerID);

private:
	Timer* FindTimer(TIMER timerID);

private:
	array<unique_ptr<Timer>, static_cast<size_t>(TIMER::COUNT)> timers{};
};

NS_END

// enum class 를 키로 사용하면 키의 종류와 개수가 컴파일 타임에 이미 확정되어 있다. 
// unordered_map은 키의 종류가 동적으로 변할 수 있는 범용적인 자료구조이지만..
// unordered_map탐색: Key -> 해시함수 -> 인덱스 -> 버킷 탐색 -> 결과 (최악의 경우 해시 충돌 처리 비용까지 발생)
// array탐색: enum 값(정수) -> 인덱스 -> 결과, 또한 데이터가 메모리에 연속적으로 배치되어 캐시 효율성도 극대화된다.