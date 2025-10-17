#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL RenderComp final : public ClonableComp<RenderComp>
{
public:
	RenderComp(Obj* owner, shared_ptr<Mesh> mesh, shared_ptr<Material> material, RENDERTYPE renderType = RENDERTYPE::NONBLEND);
	virtual ~RenderComp();

public:
	virtual void Init() override;
	shared_ptr<Mesh> GetMesh() const { return mesh; }
	shared_ptr<Material> GetMaterial() const { return material; }
	RENDERTYPE GetRenderType() const { return renderType; }

	void UpdateObjBuffer();

private:
	RENDERTYPE renderType{};
	shared_ptr<Mesh> mesh;
	shared_ptr<Material> material;
	shared_ptr<CBuffer> objBuffer;
};

NS_END