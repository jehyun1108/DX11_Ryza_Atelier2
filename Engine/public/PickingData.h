#pragma once

NS_BEGIN(Engine)

struct PickingRequest
{
	// WorldRay
	_float3 rayOrigin{};
	_float3 rayDir{};

	// ScreenRay
	bool           fromScreen = false;
	_float2        screenpos{};
	D3D11_VIEWPORT viewport{};
	Handle         cam{};

	_uint          layerMask = 0xFFFFFFFFu;
};

struct PickingHit
{
	bool     hit = false;
	float    distance = FLT_MAX;
	EntityID entity = 0;
	_float3  point{};

	// NavMesh 
	_uint triangleIdx = UINT32_MAX;
	_float2 uv{};
	_float3 normal{};
};

struct PickingData
{
	Handle      transform;
	_uint       layerMask = 0xFFFFFFFFu;
	bool        enabled    = true;

	bool        lastHit   = false;
	float       lastDist = FLT_MAX;
	_float4x4   cacheWorld{};
};

NS_END