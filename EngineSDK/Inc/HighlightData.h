#pragma once

NS_BEGIN(Engine)

enum class HighlightType : _uint
{
	None      = 0,
	Hover     = 1 << 0,
	Selected  = 1 << 1,
	Collision = 1 << 2,
};

inline _uint ToBits(HighlightType type) { return static_cast<_uint>(type); }
inline bool  HasType(_uint bits, HighlightType type) { return (bits & ToBits(type)) != 0; }

struct HighlightStyle
{
	_float4 outlineColor{ 1, 0, 0, 1 };
	float   outlinePixels = 3.f;
	_uint   priority      = 0; // 높을수록 우선
};

struct HighlightTable
{
	HighlightStyle hoverStyle     = { _float4(0.20f, 0.55f,  1.0f, 1.0f), 3.0f, 30 };  // Blue
	HighlightStyle selectedStyle  = { _float4(1.00f, 0.85f, 0.20f, 1.0f), 3.0f, 20 }; // Yellow
	HighlightStyle collisionStyle = { _float4(1.00f, 0.20f, 0.20f, 1.0f), 4.0f, 10 }; // Red
};

NS_END