#pragma once

template<typename T>
class Singleton
{
public:
	// Pass-Key 기법은 특정 함수를 호출할 수 있는 권한을 아무나 만들 수 없는 
	// 특별한 '열쇠' 타입의 객체를 인자로 요구하는 방식으로 부여한다.
	struct PassKey {};
	// T의 생성자가 예외를 던지지 않을 경우에만 noexcept가 되도록 조건부 처리
	static T& GetInstance() noexcept (is_nothrow_constructible_v<T>)
	{
		static T instance(PassKey{});
		return instance;
	}

public:
	Singleton(const Singleton&) = delete;
	Singleton& operator=(const Singleton&) = delete;
	Singleton(Singleton&&) = delete;
	Singleton& operator=(Singleton&&) = delete;

protected:
	Singleton() noexcept = default;
	virtual ~Singleton() noexcept = default;
};