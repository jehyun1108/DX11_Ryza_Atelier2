#pragma once

template<typename T>
class Singleton
{
public:
	// Pass-Key ����� Ư�� �Լ��� ȣ���� �� �ִ� ������ �ƹ��� ���� �� ���� 
	// Ư���� '����' Ÿ���� ��ü�� ���ڷ� �䱸�ϴ� ������� �ο��Ѵ�.
	struct PassKey {};
	// T�� �����ڰ� ���ܸ� ������ ���� ��쿡�� noexcept�� �ǵ��� ���Ǻ� ó��
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