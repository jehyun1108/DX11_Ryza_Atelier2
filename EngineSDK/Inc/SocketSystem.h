#pragma once

#include "SocketData.h"

NS_BEGIN(Engine)

class ENGINE_DLL SocketSystem : public EntitySystem<SocketData>, public IGuiRenderable
{
public:
	explicit SocketSystem(SystemRegistry& registry) : EntitySystem(registry) {}

	Handle Create(EntityID owner, Handle childTf, Handle parentAnim, const string& boneName,
		const _float3& offsetPos = {}, const _float3& offsetRot = {});
	Handle Create(EntityID owner, Handle childTf, Handle parentAnim, _uint boneIdx, 
		const _float3& offsetPos = {}, const _float3& offsetRot = {});

	void SetOffsetMat(Handle handle, _fmat mat);
	void SetOffsetMat(Handle handle, const _float4x4& mat);
	void SetOffsetPos(Handle handle, _float3 pos);
	void SetOffsetRot(Handle handle, _float3 euler);

	void Update(float dt);
	void RenderGui(EntityID id) override;

private:
	void RebuildOffset(SocketData& socket) const;
};

NS_END