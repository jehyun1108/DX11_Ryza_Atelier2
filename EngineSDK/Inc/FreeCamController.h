#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL FreeCamController final : public CamController
{
public:
	explicit FreeCamController(Obj* owner) : CamController(owner) {}
	virtual ~FreeCamController() = default;

public:
	virtual void Update(float dt) override;
	virtual COMPONENT GetType() const { return COMPONENT::CAMERA; }

private:
	float sensitivity = 0.5f;
};

NS_END