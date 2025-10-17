#pragma once

NS_BEGIN(Engine)

enum class RENDERPASS : uint8_t
{
	Opaque,
	Outline,
	Transparent,
	Grid,
	Debug,
	END
};

template<typename T, size_t N>
using StateArray = array<T, N>;

NS_END