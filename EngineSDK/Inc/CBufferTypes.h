#pragma once

NS_BEGIN(Engine)

struct ObjCB
{
	_float4x4 world;
	_float4   color{};
	_float4x4 invWorld;

	float     outLinePixels{};
	_float2   vpSize{};
	float     padding{};
};

struct SkyCB
{
	float   theta;
	float   opacity;
	int     isPremultiplied; // 0: Straight, 1: premultiplied
	float   padding;
};

struct TessellationCB
{
	float tsMinDist   = 10.f;
	float tsMaxDist   = 50.f;
	float tsMinFactor = 1.f;
	float tsMaxFactor = 16.f;
};

NS_END