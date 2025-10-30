#pragma once

NS_BEGIN(Engine)

struct TransformData
{
	_float3 pos{};
	_float3 scale{ 1.f, 1.f, 1.f };
	_float4 rot{};
	_float4x4 world{};
	float rotSpeed{};
	bool  dirty = true;
};

struct PlanarBasisXZ
{
	_float2 rightXZ   = {};   // (x,z) on XZ plane normalized
	_float2 forwardXZ = {}; // (x,z) on XZ plane normalized
};

NS_END