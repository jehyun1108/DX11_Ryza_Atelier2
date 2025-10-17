#pragma once

#include "TransformData.h"

NS_BEGIN(Engine)

class ENGINE_DLL TransformSystem : public EntitySystem<TransformData>, public IGuiRenderable
{
public:
	explicit TransformSystem(SystemRegistry& registry) : EntitySystem(registry) {}
	Handle Create(EntityID owner, const TransformDesc& desc);

	void Update(float dt);

	void SetPos(Handle handle, _fvec pos);
	void SetPos(Handle handle, float x, float y, float z);
	void SetScale(Handle handle, _fvec scale);
	void SetScale(Handle handle, float x, float y, float z);
	void SetEuler(Handle handle, float pitch, float yaw, float roll);
	void SetRotation(Handle handle, float yawRad, float pitchRad);
	void SetQuat(Handle handle, _fvec quat);

	void AddRotation(Handle handle, _fvec axis, float radian);
	void AddRotation(Handle handle, _fvec deltaQuat);

	void SetSpeed(Handle handle, float speed);
	void Move(Handle handle, MOVE move, float dt);
	void LookAt(Handle handle, _fvec targetPos);

	void    SetWorld(Handle handle, _fmat world);
	void    SetWorld(Handle handle, const _float4x4& world);
	_float3 GetScale(Handle handle) const;

	const _float4x4* GetWorld(Handle handle) const;

	_vec GetRight(Handle handle) const;
	_vec GetUp(Handle handle) const;
	_vec GetLook(Handle handle) const;

	void RenderGui(EntityID id) override;

private:
	void UpdateWorld(TransformData& tf);
};

NS_END