#pragma once

NS_BEGIN(Engine)

class Timer final
{
public:
	static unique_ptr<Timer> Create();

	_float GetDt() const { return dt; }

public:
	HRESULT Ready_Timer();
	void Tick();
	
private:
	LARGE_INTEGER frameTime{};
	LARGE_INTEGER fixTime{};
	LARGE_INTEGER lastTime{};
	LARGE_INTEGER cpuTick{};

	_float dt{};
};

NS_END

// LARGE_INTEGER�� 64��Ʈ ũ���� ��ȣ �ִ� ����, int �� long���� ǥ���Ҽ� ���� ū ����, ������ �ð� �����̳� ��뷮 ���� ũ�⸦ �ٷ� �� ���ȴ�.