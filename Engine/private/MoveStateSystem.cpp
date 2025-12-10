#include "Enginepch.h"

#include "NavMeshSystem.h"

void MoveStateSystem::OnBoot()
{
	tfSys      = &registry.Get<TransformSystem>();
	profileSys = &registry.Get<MoveProfileSystem>();
	intentSys  = &registry.Get<MoveIntentSystem>();
	navSys     = &registry.Get<NavMeshSystem>();
}

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
	ForEachAliveEx([&](Handle stateHandle, EntityID owner, MoveState& moveState)
		{
            Handle profileHandle{}, intentHandle{};
            MoveProfile* profile = profileSys->GetByOwner(owner, &profileHandle);
            MoveIntent* intent = intentSys->GetByOwner(owner, &intentHandle);
            if (!profile || !intent) return;

            const bool wasGrounded = moveState.grounded;

            const float speed = intent->isRunning ? profile->runSpeed : profile->walkSpeed;

            _float2 moveDir = intent->moveDir;
            if (moveDir.x != 0.f || moveDir.y != 0.f)
                moveDir = Utility::Normalize(moveDir);

            moveState.velocityXZ = { moveDir.x * speed, moveDir.y * speed };

            _float3 pos = tfSys->GetPos(moveState.tfHandle);

            // 1) XZ 이동
            pos.x += moveState.velocityXZ.x * dt;
            pos.z += moveState.velocityXZ.y * dt;

            // 2) Y 이동 (점프/중력 결과)
            pos.y += moveState.velocityY * dt;

            // 3) NavMesh에서 발 아래 높이 샘플
            _float3 navPos{};
            _float3 navNormal{};
            const bool hitNav = navSys->SampleHeight(pos, navPos, navNormal);

            moveState.hasGround = hitNav;

            bool groundedNow = false;

            if (hitNav)
            {
                const float eps = 0.01f;

                if (wasGrounded)
                {
                    // 이미 지면을 타는 중이면, 그냥 네비 높이로 따라간다
                    pos.y = navPos.y;
                    moveState.velocityY = 0.f;
                    groundedNow = true;
                }
                else if (moveState.velocityY <= 0.f && pos.y <= navPos.y + eps)
                {
                    // 공중에서 떨어져 내려오다가 닿은 프레임
                    pos.y = navPos.y;
                    moveState.velocityY = 0.f;
                    groundedNow = true;
                }

                if (groundedNow)
                {
                    moveState.groundY = navPos.y;
                    moveState.groundNormal = navNormal;
                }
            }

            moveState.prevGrounded = wasGrounded;
            moveState.grounded = groundedNow;

            tfSys->SetPos(moveState.tfHandle, XMLoadFloat3(&pos));
		});
}

void MoveStateSystem::RenderGui(EntityID id)
{
#ifdef USE_IMGUI
	/*bool any = false;
	ForEachOwned(id, [&](Handle h, MoveState& s)
		{
			any = true;
			ImGui::Separator();
			ImGui::Text("MoveState");

			float v2[2] = { s.velocityXZ.x, s.velocityXZ.y };
			ImGui::InputFloat2("velocityXZ", v2, "%.3f", ImGuiInputTextFlags_ReadOnly);
			ImGui::Text("velocityY: %.3f", s.velocityY);

			ImGui::Text("grounded: %s", s.grounded ? "true" : "false");
			float n[3] = { s.groundNormal.x, s.groundNormal.y, s.groundNormal.z };
			ImGui::Text("groundNormal: (%.3f, %.3f, %.3f)", n[0], n[1], n[2]);

			if (auto pos = tfSys->GetPos(s.tfHandle); true)
				ImGui::Text("pos: (%.3f, %.3f, %.3f)", pos.x, pos.y, pos.z);

			static bool editVy = false;
			ImGui::Checkbox("edit velocityY", &editVy);
			if (editVy)
				ImGui::DragFloat("velocityY(edit)", &s.velocityY, 0.1f, -200.f, 200.f, "%.3f");
		});

	if (!any)
		ImGui::TextDisabled("No MoveState for Entity %u", id);*/
#endif
}
