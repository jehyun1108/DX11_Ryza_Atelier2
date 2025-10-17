#pragma once

NS_BEGIN(Engine)

struct TransformData
{
	_float3 pos{};
	_float3 scale{ 1.f, 1.f, 1.f };
	_float4 rot{};
	_float4x4 world{};
	float speed{};
	float rotSpeed{};
	bool  dirty = true;
};

NS_END