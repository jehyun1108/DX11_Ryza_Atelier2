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

// enum class �� Ű�� ����ϸ� Ű�� ������ ������ ������ Ÿ�ӿ� �̹� Ȯ���Ǿ� �ִ�. 
// unordered_map�� Ű�� ������ �������� ���� �� �ִ� �������� �ڷᱸ��������..
// unordered_mapŽ��: Key -> �ؽ��Լ� -> �ε��� -> ��Ŷ Ž�� -> ��� (�־��� ��� �ؽ� �浹 ó�� ������ �߻�)
// arrayŽ��: enum ��(����) -> �ε��� -> ���, ���� �����Ͱ� �޸𸮿� ���������� ��ġ�Ǿ� ĳ�� ȿ������ �ش�ȭ�ȴ�.