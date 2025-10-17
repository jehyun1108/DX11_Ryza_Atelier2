#pragma once

NS_BEGIN(Engine)
enum class RAYORIGIN { CameraPos, NearPlane, END };

struct ENGINE_DLL CameraData
{
	Handle transform;
	Handle targetTf;
	_float3 followOffset{};

	float fovY   = XM_PIDIV4;
	float aspect = 1.777778f; // 16 : 9
	float nearZ  = 0.1f;
	float farZ = 1000.f;

	bool      isMainCam = false;
	RAYORIGIN rayPolicy = RAYORIGIN::CameraPos;

	_float4x4 view{};
	_float4x4 proj{};
	_float4x4 viewProj{};
	_float4x4 invView{};
	_float4x4 invViewProj{};
	_float4   camPos{};
};

NS_END