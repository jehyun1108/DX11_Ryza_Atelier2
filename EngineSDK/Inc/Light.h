#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL Light : public Component, public IRenderable
{
public:
	Light(Obj* owner, const LightProxy& desc = {});

public:
	virtual void Update(float dt) override;

	const LightProxy& GetData() const { return data; }

	void SetLightType(LIGHT type) { data.type = ENUM(type); }
	void SetAmbient(_fvec color) { XMStoreFloat4(&data.ambient, color); }
	void SetDiffuse(_fvec color) { XMStoreFloat4(&data.diffuse, color); }
	void SetSpecular(_fvec color) { XMStoreFloat4(&data.specular, color); }
	void SetRange(float range) { data.range = range; }
	void SetSpotAngle(float angle) { data.spotAngle = angle; }
	void SetDir(_fvec dir) { XMStoreFloat4(&data.lightDir, dir); }

	virtual void RenderGui() override;
	virtual void ExtractRenderProxies(RenderScene& out) const override;
	virtual COMPONENT GetType() const { return COMPONENT::LIGHT; }

private:
	LightProxy data{};
	Transform* tf;
};

NS_END