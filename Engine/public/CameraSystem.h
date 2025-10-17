#pragma once

#include "CameraData.h"

NS_BEGIN(Engine)

class ENGINE_DLL CameraSystem : public EntitySystem<CameraData>, public IGuiRenderable
{
public:
	explicit CameraSystem(SystemRegistry& registry) : EntitySystem(registry) {}

	Handle Create(EntityID owner, Handle transform, float fovY, float aspect, float nearZ, float farZ);

	void Update(float dt);

	void SetPerspective(Handle handle, float fovY, float aspect, float nearZ, float farZ);
	void SetTarget(Handle handle, Handle targetTf, _fvec offset);
	void ClearTarget(Handle handle);
	void SetMainCam(Handle handle, bool isMainCam);
	void SetRayPolicy(Handle handle, RAYORIGIN policy);

	Handle GetMainCamHandle() const { return mainCam; }

	// -------------- Safe -----------------
	const _float4x4* TryGetView(Handle handle)        const;
	const _float4x4* TryGetProj(Handle handle)        const;
	const _float4x4* TryGetViewProj(Handle handle)    const;
	const _float4x4* TryGetInvView(Handle handle)     const;
	const _float4x4* TryGetInvViewProj(Handle handle) const;
	_vec             TryGetPos(Handle handle)         const;
	// --------- 유효해야함 -------------------
	const _float4x4& GetView(Handle handle)        const;
	const _float4x4& GetProj(Handle handle)        const;
	const _float4x4& GetViewProj(Handle handle)    const;
	const _float4x4& GetInvView(Handle handle)     const;
	const _float4x4& GetInvViewProj(Handle handle) const;
	_vec             GetPos(Handle handle)         const;

	// ---------- 편의용 ----------------------
	const _float4x4& GetMainView()        const { return GetView(mainCam); }
	const _float4x4& GetMainProj()        const { return GetProj(mainCam); }
	const _float4x4& GetMainViewProj()    const { return GetViewProj(mainCam); }
	const _float4x4& GetMainInvView()     const { return GetInvView(mainCam); }
	const _float4x4& GetMainInvViewProj() const { return GetInvViewProj(mainCam); }
	_vec             GetMainPos()         const { return GetPos(mainCam); }

	float GetFovY(Handle handle)   const;
	float GetAspect(Handle handle) const;
	float GetNearZ(Handle handle)  const;
	float GetFarZ(Handle handle)   const;

	void CreateRayFromScreen(Handle handle, const _float2& screenPos, const D3D11_VIEWPORT& vp, _vec& outRayOrigin, _vec& outRayDir) const;
	void RenderGui(EntityID id) override;
	void ExtractCameraProxy(Handle cam, CameraProxy& out) const;

private:
	static const CameraData& RequiredCam(const CameraSystem* self, Handle handle, const char* what);

	void        UpdateFollowing(CameraData& cam, TransformSystem& tfSys, float dt) const;
	void        RebuildMatrices(CameraData& cam, TransformSystem& tfSys) const;
	static void ScreenToNdc(const _float2& screenPos, const D3D11_VIEWPORT& vp, float& ndcX, float& ndcY);

private:
	Handle               mainCam{};
};

NS_END