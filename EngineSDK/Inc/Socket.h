#pragma once

NS_BEGIN(Engine)
class Animator;

class ENGINE_DLL Socket final : public Component
{
public:
	explicit Socket(Obj* owner, Obj* parent, const string& boneName);

public:
	virtual void LateUpdate(float dt) override;
	void SetOffsetMat(const _fmat& offsetMat) { XMStoreFloat4x4(&this->offsetMat, offsetMat); }
	void SetOffsetMat(const _float4x4& offsetMat) { this->offsetMat = offsetMat; }
	void SetOffsetPos(_float3 offsetPos);
	void SetOffsetRot(_float3 offsetRot);

	virtual COMPONENT GetType() const { return COMPONENT::SOCKET; }

private:
	void UpdateOffsetMat();
	virtual void RenderGui() override;

private:
	Animator* parentAnimator{};
	_uint targetBoneIdx = -1;
	_float4x4 offsetMat;

	_float3 offsetPos{};
	_float3 offsetRot{};
};

NS_END