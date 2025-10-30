#pragma once

#include "FacingForceData.h"

NS_BEGIN(Engine)

class ENGINE_DLL FacingForceService
{
public:
	void PushSnapXZ(EntityID entity, const _float2& forwardXZ);
	void PushSmoothXZ(EntityID entity, const _float2& forwardXZ);
	void PushSmooth3D(EntityID entity, const _float3& fowardWorld);
	bool Consume(EntityID entity, FacingForceRequest& outRequest);

private:
	unordered_map<EntityID, FacingForceRequest> requestByEntity;
};

NS_END