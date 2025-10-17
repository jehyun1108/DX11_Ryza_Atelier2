#pragma once

NS_BEGIN(Engine)

struct ENGINE_DLL IReservable
{
	virtual ~IReservable() = default;
	virtual void Reserve(size_t n) = 0;
};

NS_END