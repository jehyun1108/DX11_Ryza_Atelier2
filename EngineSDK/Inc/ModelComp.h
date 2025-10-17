#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL ModelComp final : public Component, public IRenderable
{
public:
	ModelComp(Obj* owner, const wstring& modelKey);

public:
	shared_ptr<Model> GetModel() const { return model; }

	virtual void RenderGui() override;

	virtual void ExtractRenderProxies(RenderScene& out) const override;

	virtual COMPONENT GetType() const { return COMPONENT::MODEL; }

private:
	shared_ptr<Model> model;
};

NS_END