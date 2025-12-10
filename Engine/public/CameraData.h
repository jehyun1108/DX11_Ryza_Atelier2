#pragma once

NS_BEGIN(Engine)
enum class RAYORIGIN      { CameraPos,   NearPlane, END };
enum class OffsetSpace    { TargetSpace, WorldSpace };
enum class FollowPolicy   { HardLookAt,  SoftLookAt, PosOnly };
enum class ProjectionType { Perspective, Orthographic };
struct CameraData
{
	EntityID owner{};
	Handle   transform;
	Handle   targetTf;

	ProjectionType projType = ProjectionType::Perspective;
	float orthoWidth = 0.f;
	float orthoHeight = 0.f;

	float fovY{}, aspect{}, nearZ{}, farZ{};
	_float3 followOffset{};
	_float3 lookAtOffset{};

	OffsetSpace  offsetSpace  = OffsetSpace::TargetSpace;  
	FollowPolicy followPolicy = FollowPolicy::HardLookAt;   
	float        softDamping  = 10.f;

	_float4 desiredRot{};  
	bool    desiredInit = false;

	_float4x4 view{}, proj{}, viewProj{}, invView{}, invViewProj{}, invProj{};
	_float4   forward{};
	_float4   camPos{};
	bool      isMainCam = false;
	RAYORIGIN rayPolicy = RAYORIGIN::CameraPos;
};

struct DebugCamState
{
	bool  enabled = false;
	Handle cam{};
	float moveSpeed = 800.f;
	float rotSpeed = 0.15f;
	float yawDeg = 0.f;
	float pitchDeg = 0.f;
};

NS_END