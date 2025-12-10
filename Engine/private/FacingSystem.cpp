#include "Enginepch.h"

void FacingSystem::OnBoot()
{
	tfSys     = &registry.Get<TransformSystem>();
	moveSys   = &registry.Get<MoveStateSystem>();
	intentSys = &registry.Get<MoveIntentSystem>();
	forceSrv  = &registry.Get<FacingForceService>();
	blockSrv  = &registry.Get<FacingBlockService>();
}

Handle FacingSystem::Create(EntityID owner)
{
	Handle handle = CreateComp(owner);
	if (auto comp = Get(handle))
		*comp = FacingComponent{};
	return handle;
}

void FacingSystem::Update(float dt)
{
	ForEachAliveEx([&](Handle handle, EntityID owner, FacingComponent& comp)
		{
			const MoveState* moveState = moveSys->GetByOwner(owner);
			if (!moveState || blockSrv->IsBlocked(owner)) return;

			const FacingParams& params = comp.params;
			const float maxStepRad = XMConvertToRadians(params.turnSpeed) * dt;

			// 1. Force 우선
			FacingForceRequest request{};
			if (forceSrv->Consume(owner, request))
			{
				if (request.lockXZ)
				{
					_float3 dir = { request.forward.x, 0.f, request.forward.z };
					const float lenSq = dir.x * dir.x + dir.z * dir.z;
					if (lenSq > 1e-12f)
					{
						if (request.snap)
							tfSys->SetForward(moveState->tfHandle, dir);
						else
						{
							const _float2 curXZ = tfSys->GetForwardXZ(moveState->tfHandle);
							const _float2 dstXZ = Utility::Normalize(_float2{ dir.x, dir.z });
							const   float delta = Utility::SignedAngRad2D(curXZ, dstXZ);
							const   float apply = Utility::Clamp(delta, -maxStepRad, maxStepRad);
							const _float2 newXZ = Utility::Rotate2D(curXZ, apply);
							tfSys->SetForward(moveState->tfHandle, _float3{ newXZ.x, 0.f, newXZ.y });
						}
					}
				}
				else
					tfSys->SetForward(moveState->tfHandle, request.forward);

				return;
			}

			// 2. Input -> 회전
			const MoveIntent* moveIntent = intentSys->GetByOwner(owner);
			if (!moveIntent) return;

			const float lenSq = moveIntent->moveDir.x * moveIntent->moveDir.x + moveIntent->moveDir.y * moveIntent->moveDir.y;
			if (lenSq <= 1e-12f) return;

			_float2 dstXZ = Utility::Normalize(_float2{ moveIntent->moveDir.x, moveIntent->moveDir.y });

			if (fabsf(params.forwardOffsetRad) > 1e-6f)
				dstXZ = Utility::Rotate2D(dstXZ, params.forwardOffsetRad);

			const _float2 curXZ = tfSys->GetForwardXZ(moveState->tfHandle);
			const   float delta = Utility::SignedAngRad2D(curXZ, dstXZ);
			const   float apply = Utility::Clamp(delta, -maxStepRad, maxStepRad);
			const _float2 newXZ = Utility::Rotate2D(curXZ, apply);

			tfSys->SetForward(moveState->tfHandle, _float3{ newXZ.x, 0.f, newXZ.y });
		});
}