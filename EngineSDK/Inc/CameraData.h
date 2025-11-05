#pragma once

NS_BEGIN(Engine)
enum class RAYORIGIN    { CameraPos,   NearPlane, END };
enum class OffsetSpace  { TargetSpace, WorldSpace };
enum class FollowPolicy { HardLookAt,  SoftLookAt, PosOnly };

struct ENGINE_DLL CameraData
{
	EntityID owner{};
	Handle transform;
	Handle targetTf;

	float fovY{}, aspect{}, nearZ{}, farZ{};
	_float3 followOffset = {};

	OffsetSpace  offsetSpace = OffsetSpace::TargetSpace;  
	FollowPolicy followPolicy = FollowPolicy::HardLookAt;   
	float        softDamping = 10.f;

	// 내부 캐시(소프트 회전용)
	_float4 desiredRot{};  
	bool    desiredInit = false;

	_float4x4 view{}, proj{}, viewProj{}, invView{}, invViewProj{};
	_float4   camPos{};
	bool      isMainCam = false;
	RAYORIGIN rayPolicy = RAYORIGIN::CameraPos;
};

NS_END