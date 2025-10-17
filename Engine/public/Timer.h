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

// LARGE_INTEGER은 64비트 크기의 부호 있는 정수, int 나 long으로 표현할수 없는 큰 숫자, 정밀한 시간 측정이나 대용량 파일 크기를 다룰 때 사용된다.