#include "Enginepch.h"

Handle JumpSystem::Create(EntityID owner)
{
	Handle handle = CreateComp(owner);
	return handle;
}

void JumpSystem::Priority_Update(float dt)
{
	auto& inputService = registry.Get<InputService>();
	auto& moveSys      = registry.Get<MoveStateSystem>();

	ForEachAliveEx([&](Handle handle, EntityID owner, JumpComponent& jump)
		{
			MoveState* moveState = moveSys.GetByOwner(owner);
			if (!moveState) return;

			const bool jumpEdgeNow = inputService.ConsumeJumpEdge(owner);

			if (jumpEdgeNow && moveState->grounded)
				moveState->velocityY = jumpParams.jumpSpeed;

			if (!moveState->grounded)
			{
				moveState->velocityY += jumpParams.gravity * dt;
				if (moveState->velocityY < -jumpParams.maxFallSpeed)
					moveState->velocityY = -jumpParams.maxFallSpeed;
			}
		});
}

void JumpSystem::Update(float dt)
{

}