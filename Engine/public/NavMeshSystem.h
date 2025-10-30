#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL NavMeshSystem : public EntitySystem<NavMeshComponent>, public IGuiRenderable
{
public:
	explicit NavMeshSystem(SystemRegistry& registry) :EntitySystem(registry) {}

	void SetRenderParams(Handle handle, const NavMeshRenderParams& params);
	const NavMeshRenderParams* GetRenderParams() const;

	void SetPreviewBoundary(Handle handle, const vector<BoundaryEdge>& edges);
	void ClearPreviewBoundary(Handle handle);
	// Dirty
	void MarkDirty(Handle handle);
	void MarkDirtyByOwner(EntityID owner);
	// PipeLine
	void Build(Handle handle);
	void Render(Handle handle, ID3D11DeviceContext* context);
	void RenderAll(ID3D11DeviceContext* context);
	void Update(float dt);

	void RenderGui(EntityID id) override;
};

NS_END