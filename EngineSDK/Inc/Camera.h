#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL Camera final : public Component, public IRenderable
{
public:
	Camera(Obj* owner);

public:
	virtual void LateUpdate(float dt) override;

	_mat GetViewMat()         const { return XMLoadFloat4x4(&viewMat); }
	_mat GetProjMat()         const { return XMLoadFloat4x4(&projMat); }
	_mat GetViewProjMat()     const { return XMLoadFloat4x4(&viewProjMat); }
	_mat GetInvViewMat()      const { return XMLoadFloat4x4(&invViewMat); }
	_mat GetInvViewProjMat()  const { return XMLoadFloat4x4(&invViewProjMat); }

	float GetFovY()   const { return fovY; }
	float GetAspect() const { return aspect; }
	float GetNearZ()  const { return nearZ; }
	float GetFarZ()   const { return farZ; }

	void SetPerspective(float _fovY, float _aspect, float _nearZ, float _farZ);
	void SetTarget(Obj* target, _fvec offset = { 0.f, 2.f, -2.f });

	void SetMainCam(bool isMainCam) { this->isMainCam = isMainCam; }
	_vec GetPos() const { return XMLoadFloat4(&camPos); }

	void CreateRayFromScreen(const _float2& screenPos, _vec& outRayOrigin, _vec& outRayDir) const;

	void SetRayPolicy(RAYORIGIN policy) { rayPolicy = policy; }

	virtual void RenderGui() override;
	virtual void ExtractRenderProxies(RenderScene& out) const override;
	virtual COMPONENT GetType() const { return COMPONENT::CAMERA; }

private:
	void UpdateMatrices();
	void ScreenToNdc(const _float2& screenPos, const D3D11_VIEWPORT& vp, float& outX, float& outY) const;

private:
	_float4x4 viewMat{};
	_float4x4 projMat{};
	_float4x4 viewProjMat{};
	_float4x4 invViewMat{};
	_float4x4 invViewProjMat{};
	_float4   camPos{};

	Obj*    targetObj{};
	_float3 offset{};

	float fovY{};
	float aspect{};
	float nearZ{};
	float farZ{};

	bool isMainCam = false;

	RAYORIGIN rayPolicy = RAYORIGIN::CameraPos;
};

NS_END