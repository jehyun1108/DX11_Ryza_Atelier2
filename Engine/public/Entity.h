#pragma once

NS_BEGIN(Engine)

using EntityID = _uint;
constexpr EntityID invalidEntity = 0;

struct ENGINE_DLL Handle
{
	static constexpr _uint invalidIdx = 0xFFFFFFFFu;
	_uint idx = invalidIdx;
	_uint generation = 0;

	bool IsValid() const { return idx != invalidIdx; }
};

inline constexpr bool operator==(const Handle& a, const Handle& b){ return a.idx == b.idx && a.generation == b.generation; }
inline constexpr bool operator!=(const Handle& a, const Handle& b) noexcept { return !(a == b);}

NS_END