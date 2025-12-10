#pragma once

#include "OrbitCamData.h"

NS_BEGIN(Engine)

class ENGINE_DLL OrbitCamSystem : public EntitySystem<OrbitCamData>
{
public:
	explicit OrbitCamSystem(SystemRegistry& registry) : EntitySystem(registry) {}
	void     OnBoot() override;
	Handle   Create(EntityID owner, Handle camHandle, Handle targetTf);

	void Update(float dt);

private:
	InputService* input{};
	CameraSystem* camSys{};
};

NS_END