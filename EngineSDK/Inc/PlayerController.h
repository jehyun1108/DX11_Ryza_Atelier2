#pragma once

#include "PlayerControllerData.h"

NS_BEGIN(Engine)

class ENGINE_DLL PlayerController : public IGuiRenderable
{
public:
	explicit PlayerController(SystemRegistry& registry, InputService& inputService, EntityID playerID, Handle camTf)
		:registry(registry), inputService(inputService) ,playerID(playerID), camTf(camTf) {}

	EntityID GetPlayerID() const { return playerID; }

	void Update(float dt);
	void RenderGui(EntityID id) override;

private:
	SystemRegistry& registry;
	InputService& inputService;
	EntityID playerID{};
	Handle   camTf{};
	bool     prevJumpDown = false;
	bool     prevAttackDown = false;
};

NS_END