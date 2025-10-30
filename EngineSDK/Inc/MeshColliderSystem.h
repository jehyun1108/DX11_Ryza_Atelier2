#pragma once

#include "MeshColliderData.h"

NS_BEGIN(Engine)

class ENGINE_DLL MeshColliderSystem : public EntitySystem<MeshColliderData>
{
public:
	explicit MeshColliderSystem(SystemRegistry& registry) : EntitySystem(registry) {}
	Handle Create(EntityID owner, Handle tfHandle, const Model& model, _uint layerMask = 0xFFFFFFFFu, bool enabled = true);
	const MeshColliderData* TryGetByOwner(EntityID owner, Handle* outHandle = nullptr) const { return GetByOwner(owner, outHandle); }

private:
	void ExtractCPUFromModel(const Model& model, vector<_float3>& outPos, vector<_uint>& outIndices);
	bool CopyPosAndIndicesFromMesh(const Mesh& mesh, vector<_float3>& outPos, vector<_uint>& outIndices);
};

NS_END