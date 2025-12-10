#pragma once

NS_BEGIN(Engine)

struct ParticleInstance
{
	_float3 pos;
	float   size;
	_float4 color;
	float   rotRad;

	_float2 uvMin;
	_float2 uvMax;
};
struct ParticleVertex
{
	_float3 localPos;
	_float2 uv;
};

NS_END