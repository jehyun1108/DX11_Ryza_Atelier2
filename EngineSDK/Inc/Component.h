#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL Component abstract
{
	friend class ComponentRegistry;

protected:
	Component(Obj* owner) : owner(owner) {}

public:
	virtual ~Component() = default;

	virtual void PriorityUpdate(float) {}
	virtual void Update(float dt) {}
	virtual void LateUpdate(float dt) {}

	virtual COMPONENT GetType() const { return COMPONENT::NONE; }

	virtual void RenderGui() {}

	Obj* GetOwner() const { return owner; }
	void SetOwner(Obj* obj) { owner = obj; }

protected:
	Obj* owner{};
	GameInstance& game = GameInstance::GetInstance();

private:
	bool isRegistered = false;
	bool pendingAdd = false;
	bool pendingRem = false;
	size_t registIdx = SIZE_MAX; // bucket 에서 위치한 idx
};

NS_END