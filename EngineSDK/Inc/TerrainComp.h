#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL TerrainComp final : public Component, public IRenderable
{
public:
	TerrainComp(Obj* owner, shared_ptr<StaticMesh> staticMesh, shared_ptr<Material> material);

	virtual void ExtractRenderProxies(RenderScene& scene) const override;

	virtual void RenderGui() override;

	virtual COMPONENT GetType() const { return COMPONENT::TERRAIN; }

private:
	shared_ptr<StaticMesh> staticMesh;
	shared_ptr<Material> material;
};

NS_END