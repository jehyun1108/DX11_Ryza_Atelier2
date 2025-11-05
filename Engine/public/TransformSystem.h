#pragma once

#include "TransformData.h"

NS_BEGIN(Engine)

// 결과를 좌표계에 반영 (행렬 갱신)
class ENGINE_DLL TransformSystem : public EntitySystem<TransformData>, public IGuiRenderable
{
public:
	explicit TransformSystem(SystemRegistry& registry) : EntitySystem(registry) {}
	Handle   Create(EntityID owner, const TransformDesc& desc);

	void Update(float dt);

	void SetPos(Handle handle, _fvec pos);
	void SetPos(Handle handle, float x, float y, float z);
	void SetPos(Handle handle, _float3 pos);
	_float3 GetPos(Handle handle) const;

	void SetScale(Handle handle, _fvec scale);
	void SetScale(Handle handle, float x, float y, float z);
	void SetEuler(Handle handle, float pitch, float yaw, float roll);
	void SetRotation(Handle handle, float yawRad, float pitchRad); 
	void SetQuat(Handle handle, _fvec quat);
	void SetForwardXZ(Handle handle, const _float3& worldPos);
	void SetForward(Handle handle, const _float3& worldPos, const _float3& upWorld = _float3{ 0.f, 1.f, 0.f });
	
	_float2 GetForwardXZ(Handle handle) const;
	_float3 GetForward(Handle handle) const;

	_float4 GetRot(Handle handle) const;

	void AddWorldOffset(Handle handle, const _float3& dtWorld);
	void AddLocalOffset(Handle handle, const _float3& dtLocal);

	void    LookAt(Handle handle, _fvec targetPos);

	void    SetWorld(Handle handle, _fmat world);
	void    SetWorld(Handle handle, const _float4x4& world);
	_float3 GetScale(Handle handle) const;

	const _float4x4* GetWorld(Handle handle) const;
	PlanarBasisXZ    GetPlanarBasisXZ(Handle handle) const;

	_vec GetRight(Handle handle) const;
	_vec GetUp(Handle handle) const;
	_vec GetLook(Handle handle) const;

	void RenderGui(EntityID id) override;

private:
	void UpdateWorld(TransformData& tf);
};

NS_END