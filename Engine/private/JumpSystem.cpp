#include "Enginepch.h"
#include "JumpSystem.h"

void JumpSystem::OnBoot()
{
	input   = &registry.Get<InputService>();
	moveSys = &registry.Get<MoveStateSystem>();
}

Handle JumpSystem::Create(EntityID owner)
{
	Handle handle = CreateComp(owner);
	return handle;
}

void JumpSystem::Priority_Update(float dt)
{
	ForEachAliveEx([&](Handle handle, EntityID owner, JumpComponent& jump)
		{
			MoveState* moveState = moveSys->GetByOwner(owner);
			const bool jumpEdgeNow = input->ConsumeJumpEdge(owner);

			if (jumpEdgeNow && moveState->grounded)
			{
				moveState->velocityY = jumpParams.jumpSpeed;
				moveState->grounded = false;
			}

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