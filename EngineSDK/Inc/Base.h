#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL Base abstract
{
protected:
	Base() {}
	virtual ~Base() = default;

public:
	_uint AddRef();
	_uint Release();

public:
	virtual void Free() {}

protected:
	_uint refCnt{};
};

NS_END