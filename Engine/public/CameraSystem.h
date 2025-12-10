#pragma once

#include "CameraData.h"

NS_BEGIN(Engine)

class ENGINE_DLL CameraSystem : public EntitySystem<CameraData>, public IGuiRenderable
{
public:
	explicit CameraSystem(SystemRegistry& registry) : EntitySystem(registry) {}
	void     OnBoot() override;
	Handle   Create(EntityID owner, Handle transform, float fovY, float aspect, float nearZ, float farZ);
	void     Update(float dt);

	void SetPerspective(Handle handle, float fovY, float aspect, float nearZ, float farZ);
	void SetOrtho(Handle handle, float width, float height, float nearZ, float farZ);

	void SetTarget(Handle handle, Handle targetTf, _fvec offset);
	void ClearTarget(Handle handle);
	void SetMainCam(Handle handle, bool isMainCam);
	void SetRayPolicy(Handle handle, RAYORIGIN policy);
	void SetFollowOffsetSpace(Handle handle, OffsetSpace space);
	void SetFollowPolicy(Handle handle, FollowPolicy policy, float softDamping = 10.f);
	void SetLookAtOffset(Handle handle, _fvec offset);

	Handle GetMainCamHandle() const { return mainCam; }

	const _float4x4& GetView(Handle handle)        const { return RequiredCam(this, handle, "GetView: invalid camera").view; }
	const _float4x4& GetProj(Handle handle)        const { return RequiredCam(this, handle, "GetProj: invalid camera").proj; }
	const _float4x4& GetViewProj(Handle handle)    const { return RequiredCam(this, handle, "GetViewProj: invalid camera").viewProj; }
	const _float4x4& GetInvView(Handle handle)     const { return RequiredCam(this, handle, "GetInvView: invalid camera").invView; }
	const _float4x4& GetInvViewProj(Handle handle) const { return RequiredCam(this, handle, "GetInvViewProj: invalid camera").invViewProj; }
	const _float4x4& GetInvProj(Handle handle)     const { return RequiredCam(this, handle, "GetInvProj: invalid camera").invProj; }
	_vec             GetPos(Handle handle)         const { return XMLoadFloat4(&RequiredCam(this, handle, "GetPos: invalid camera").camPos); }
	_vec             GetForward(Handle handle)     const { return XMLoadFloat4(&RequiredCam(this, handle, "GetForward: invalid camera").forward); }

	const _float4x4& GetMainView()        const { return GetView(mainCam);        }
	const _float4x4& GetMainProj()        const { return GetProj(mainCam);        }
	const _float4x4& GetMainViewProj()    const { return GetViewProj(mainCam);    }
	const _float4x4& GetMainInvView()     const { return GetInvView(mainCam);     }
	const _float4x4& GetMainInvProj()     const { return GetInvProj(mainCam);     }
	const _float4x4& GetMainInvViewProj() const { return GetInvViewProj(mainCam); }
	_vec             GetMainPos()         const { return GetPos(mainCam);         }
	_vec             GetMainForward()     const { return GetForward(mainCam); }

	float GetFovY(Handle handle)   const { return Validate(handle) ? Get(handle)->fovY   : 0.f; }
	float GetAspect(Handle handle) const { return Validate(handle) ? Get(handle)->aspect : 0.f; }
	float GetNearZ(Handle handle)  const { return Validate(handle) ? Get(handle)->nearZ  : 0.f; }
	float GetFarZ(Handle handle)   const { return Validate(handle) ? Get(handle)->farZ   : 0.f; }

	void CreateRayFromScreen(Handle handle, const _float2& screenPos, const D3D11_VIEWPORT& vp, _vec& outRayOrigin, _vec& outRayDir) const;
	void RenderGui(EntityID id) override;
	void ExtractCameraProxy(Handle cam, CameraProxy& out) const;

private:
	static const CameraData& RequiredCam(const CameraSystem* self, Handle handle, const char* what);

	void        UpdateFollowing(CameraData& cam, float dt) const;
	void        RebuildMatrices(CameraData& cam) const;
	static void ScreenToNdc(const _float2& screenPos, const D3D11_VIEWPORT& vp, float& ndcX, float& ndcY);

private:
	TransformSystem*     tfSys{};
	Handle               mainCam{};
};

NS_END