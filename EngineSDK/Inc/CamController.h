#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL CamController abstract : public Component
{
public:
	explicit CamController(Obj* owner) : Component(owner) {}
	virtual ~CamController() = default;

public:
	void SetActive(bool isActive) { this->isActive = isActive; }

protected:
	bool isActive = true;
};

NS_END