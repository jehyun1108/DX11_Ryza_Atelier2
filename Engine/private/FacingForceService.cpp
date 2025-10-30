#include "Enginepch.h"

void FacingForceService::PushSnapXZ(EntityID entity, const _float2& forwardXZ)
{
	FacingForceRequest& request = requestByEntity[entity];
	request.has     = true;
	request.forward = _float3{ forwardXZ.x, 0.f, forwardXZ.y };
	request.lockXZ  = true;
	request.snap    = true;
}

void FacingForceService::PushSmoothXZ(EntityID entity, const _float2& forwardXZ)
{
	FacingForceRequest& request = requestByEntity[entity];
	request.has     = true;
	request.forward = _float3{ forwardXZ.x, 0.f, forwardXZ.y };
	request.lockXZ  = true;
	request.snap    = false;
}

void FacingForceService::PushSmooth3D(EntityID entity, const _float3& fowardWorld)
{
	FacingForceRequest& request = requestByEntity[entity];
	request.has     = true;
	request.forward = fowardWorld;
	request.lockXZ  = false;
	request.snap    = false;
}

bool FacingForceService::Consume(EntityID entity, FacingForceRequest& outRequest)
{
	auto it = requestByEntity.find(entity);
	if (it == requestByEntity.end() || !it->second.has) return false;
	outRequest = it->second;
	it->second.has = false;
	return true;
}