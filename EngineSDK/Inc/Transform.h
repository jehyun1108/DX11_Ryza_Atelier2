#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL Transform final : public Component
{
public:
	explicit Transform(Obj* owner, const TransformDesc& desc = {});
	virtual ~Transform() = default;

public:
	_vec GetPos()   const { return XMLoadFloat3(&pos); } 
	_vec GetScale() const { return XMLoadFloat3(&scale); }
	_vec GetRot()   const { return XMLoadFloat4(&rot); }

	_vec GetRight() const;
	_vec GetUp()    const;
	_vec GetLook()  const;

	void SetPos(_fvec pos);
	void SetPos(float x, float y, float z = 0.f);

	void SetScale(_fvec scale);
	void SetScale(float x, float y, float z = 1.f);

	void SetSpeed(float speed) { this->speed = speed; }

	void Move(MOVE state, float dt);
	void LookAt(_fvec targetPos);
	_mat GetWorldMat() const { return XMLoadFloat4x4(&worldMat); }

	void SetWorldMat(const _fmat& worldMat);

	virtual void RenderGui() override;
	virtual void LateUpdate(float dt) override;

	virtual COMPONENT GetType() const { return COMPONENT::TRANSFORM; }

public:
	void SetRotationQuat(_fvec quat);
	void SetEuler(float pitch, float yaw, float roll);
	void AddRotation(_fvec axis, float radian);

private:
	void UpdateWorldMat();
	_float3 GetEulerAngles();

private:
	_float     speed{};
	_float     rotSpeed{};

	_float3       pos{};
	_float3       scale = {1.f, 1.f, 1.f};
	_float4       rot{};

    _float4x4  worldMat{};
    bool dirty = true;
};

NS_END