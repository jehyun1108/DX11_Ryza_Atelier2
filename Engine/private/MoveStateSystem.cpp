#include "Enginepch.h"

Handle MoveStateSystem::Create(EntityID owner, Handle tfHandle)
{
	Handle stateHandle  = CreateComp(owner);
	auto& state         = *Get(stateHandle);
	state.tfHandle      = tfHandle;
	state.velocityXZ    = { 0.f, 0.f };
	state.velocityY     = 0.f;
	state.grounded      = false;
	state.groundNormal  = { 0.f, 1.f, 0.f };
	return stateHandle;
}

void MoveStateSystem::Update(float dt)
{
	auto& tfSys      = registry.Get<TransformSystem>();
	auto& profileSys = registry.Get<MoveProfileSystem>();
	auto& intentSys  = registry.Get<MoveIntentSystem>();

	ForEachAliveEx([&](Handle stateHandle, EntityID owner, MoveState& moveState)
		{
			Handle profileHandle{}, intentHandle{};
			MoveProfile* profile  = profileSys.GetByOwner(owner, &profileHandle);
			MoveIntent*  intent   = intentSys.GetByOwner(owner, &intentHandle);
			if (!profile || !intent) return;

			const float speed = intent->isRunning ? profile->runSpeed : profile->walkSpeed;

			_float2 moveDir = intent->moveDir;
			if (moveDir.x != 0.f || moveDir.y != 0.f) 
				moveDir = Utility::Normalize(moveDir);
			else                                     
				moveDir = _float2{ 0.f, 0.f };

			moveState.velocityXZ = _float2{ moveDir.x * speed, moveDir.y * speed };

			const _float3 worldOffset =
			{
				moveState.velocityXZ.x * dt,
				moveState.velocityY * dt,
				moveState.velocityXZ.y * dt
			};
			tfSys.AddWorldOffset(moveState.tfHandle, worldOffset);

			_float3 worldPos = tfSys.GetPos(moveState.tfHandle);

			if (worldPos.y <= 0.f)
			{
				worldPos.y = 0.f;
				tfSys.SetPos(moveState.tfHandle, XMLoadFloat3(&worldPos));
				moveState.velocityY = 0.f;
				moveState.grounded = true;
				moveState.groundNormal = { 0.f, 1.f, 0.f };
			}
			else
				moveState.grounded = false;
		});
}

void MoveStateSystem::RenderGui(EntityID filterID)
{
#ifdef USE_IMGUI
	
	
#endif
}
