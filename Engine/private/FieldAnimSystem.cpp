#include "Enginepch.h"

Handle FieldAnimSystem::Create(EntityID owner, Handle animHandle)
{
    AnimProfile profile{};
    profile.character = CharacterID::Ryza;
    profile.context = AnimContext::Field;
    return Create(owner, animHandle, profile);
}

Handle FieldAnimSystem::Create(EntityID owner, Handle animHandle, const AnimProfile& profile)
{
    Handle handle = CreateComp(owner);
    auto& data = *Get(handle);

    data.animHandle   = animHandle;
    data.layerIdx     = 0;
    data.cur          = LocomotionState::Idle;
    data.stateElapsed = 0.f;
    data.profile      = profile;

    auto& animSys = registry.Get<AnimatorSystem>();
    animSys.SetLayerBlendWeight(animHandle, 0, 1.f);
    animSys.SetLayerBlendType(animHandle, 0, ANIMBLEND::OVERRIDE);
    
    PlayKey(data, AnimKey::Idle, ANIMTYPE::LOOP, 0.f);
    return handle;
}

void FieldAnimSystem::Update(float dt)
{
    auto& moveSys      = registry.Get<MoveStateSystem>();
    auto& intentSys    = registry.Get<MoveIntentSystem>();
    auto& inputService = registry.Get<InputService>();

    ForEachAliveEx([&](Handle handle, EntityID owner, LocomotionAnim& loco)
        {
            MoveState* move = moveSys.GetByOwner(owner);
            const MoveIntent* intent = intentSys.GetByOwner(owner);
            if (!move) return;

            const float speedXZ    = sqrtf(move->velocityXZ.x * move->velocityXZ.x + move->velocityXZ.y * move->velocityXZ.y);
            const float velocityY  = move->velocityY;
            const bool  isGrounded = move->grounded;

            const bool  runInput   = intent ? intent->isRunning : (speedXZ >= params.runStartThreshold);

            loco.stateElapsed += dt;

            const bool justLanded     = ( isGrounded && !loco.wasGroundedPrev);
            const bool justLeftGround = (!isGrounded &&  loco.wasGroundedPrev);

            const bool attackEdgeNow = inputService.ConsumeAttackEdge(owner);

            if (justLeftGround)
            {
                if (velocityY > 0.f)
                {
                    loco.cur = LocomotionState::JumpStart;
                    PlayKey(loco, AnimKey::JumpStart, ANIMTYPE::ONCE, params.fadeShort);
                }
                else
                {
                    loco.cur = LocomotionState::JumpLoop;
                    PlayKey(loco, AnimKey::JumpLoop, ANIMTYPE::LOOP, params.fadeShort);
                }
            }

            switch (loco.cur)
            {
            case LocomotionState::Idle:
            {
                if (attackEdgeNow && isGrounded)
                {
                    loco.cur = LocomotionState::FieldSwing;
                    PlayKey(loco, AnimKey::FieldSwing, ANIMTYPE::ONCE, params.fadeShort);
                    break;
                }

                if (!isGrounded)
                {
                    loco.cur = (velocityY > 0.f) ? LocomotionState::JumpStart : LocomotionState::JumpLoop;
                    PlayKey(loco, (velocityY > 0.f) ? AnimKey::JumpStart : AnimKey::JumpLoop,
                        (velocityY > 0.f) ? ANIMTYPE::ONCE : ANIMTYPE::LOOP, params.fadeShort);
                    break;
                }

                if (speedXZ > params.runStartThreshold && runInput)
                {
                    loco.cur = LocomotionState::RunStart;
                    PlayKey(loco, AnimKey::RunStart, ANIMTYPE::ONCE, params.fadeShort);
                }
                else if (speedXZ > params.walkStartThreshold)
                {
                    loco.cur = LocomotionState::WalkStart;
                    PlayKey(loco, AnimKey::WalkStart, ANIMTYPE::ONCE, params.fadeShort);
                }
                break;
            }

            case LocomotionState::WalkStart:
            {
                if (!isGrounded)
                {
                    loco.cur = (velocityY > 0.f) ? LocomotionState::JumpStart : LocomotionState::JumpLoop;
                    PlayKey(loco, (velocityY > 0.f) ? AnimKey::JumpStart : AnimKey::JumpLoop,
                        (velocityY > 0.f) ? ANIMTYPE::ONCE : ANIMTYPE::LOOP, params.fadeShort);
                    break;
                }

                if (IsCurClipFinished(loco))
                {
                    loco.cur = LocomotionState::WalkLoop;
                    PlayKey(loco, AnimKey::WalkLoop, ANIMTYPE::LOOP, params.fadeShort);
                }
                else if (runInput && speedXZ > params.runStartThreshold)
                {
                    loco.cur = LocomotionState::RunStart;
                    PlayKey(loco, AnimKey::RunStart, ANIMTYPE::ONCE, params.fadeShort);
                }
                else if (speedXZ < params.walkStopThreshold)
                {
                    loco.cur = LocomotionState::WalkEnd;
                    PlayKey(loco, AnimKey::WalkEnd, ANIMTYPE::ONCE, params.fadeShort);
                }
                break;
            }

            case LocomotionState::WalkLoop:
            {
                if (!isGrounded)
                {
                    loco.cur = (velocityY > 0.f) ? LocomotionState::JumpStart : LocomotionState::JumpLoop;
                    PlayKey(loco, (velocityY > 0.f) ? AnimKey::JumpStart : AnimKey::JumpLoop,
                        (velocityY > 0.f) ? ANIMTYPE::ONCE : ANIMTYPE::LOOP, params.fadeShort);
                    break;
                }

                if (runInput && speedXZ > params.runStartThreshold)
                {
                    loco.cur = LocomotionState::RunStart;
                    PlayKey(loco, AnimKey::RunStart, ANIMTYPE::ONCE, params.fadeShort);
                }
                else if (speedXZ < params.walkStopThreshold)
                {
                    loco.cur = LocomotionState::WalkEnd;
                    PlayKey(loco, AnimKey::WalkEnd, ANIMTYPE::ONCE, params.fadeShort);
                }
                break;
            }

            case LocomotionState::WalkEnd:
            {
                if (!isGrounded)
                {
                    loco.cur = (velocityY > 0.f) ? LocomotionState::JumpStart : LocomotionState::JumpLoop;
                    PlayKey(loco, (velocityY > 0.f) ? AnimKey::JumpStart : AnimKey::JumpLoop,
                        (velocityY > 0.f) ? ANIMTYPE::ONCE : ANIMTYPE::LOOP, params.fadeShort);
                    break;
                }

                if (IsCurClipFinished(loco))
                {
                    if (speedXZ > params.runStartThreshold && runInput)
                    {
                        loco.cur = LocomotionState::RunStart;
                        PlayKey(loco, AnimKey::RunStart, ANIMTYPE::ONCE, params.fadeShort);
                    }
                    else if (speedXZ > params.walkStartThreshold)
                    {
                        loco.cur = LocomotionState::WalkStart;
                        PlayKey(loco, AnimKey::WalkStart, ANIMTYPE::ONCE, params.fadeShort);
                    }
                    else
                    {
                        loco.cur = LocomotionState::Idle;
                        PlayKey(loco, AnimKey::Idle, ANIMTYPE::LOOP, params.fadeShort);
                    }
                }
                break;
            }

            case LocomotionState::RunStart:
            {
                if (!isGrounded)
                {
                    loco.cur = (velocityY > 0.f) ? LocomotionState::JumpStart : LocomotionState::JumpLoop;
                    PlayKey(loco, (velocityY > 0.f) ? AnimKey::JumpStart : AnimKey::JumpLoop,
                        (velocityY > 0.f) ? ANIMTYPE::ONCE : ANIMTYPE::LOOP, params.fadeShort);
                    break;
                }

                if (IsCurClipFinished(loco))
                {
                    loco.cur = LocomotionState::RunLoop;
                    PlayKey(loco, AnimKey::RunLoop, ANIMTYPE::LOOP, params.fadeShort);
                }
                else if ((!runInput && speedXZ < params.runStopThreshold) || speedXZ < params.walkStopThreshold)
                {
                    loco.cur = LocomotionState::RunEnd;
                    PlayKey(loco, AnimKey::RunEnd, ANIMTYPE::ONCE, params.fadeShort);
                }
                break;
            }

            case LocomotionState::RunLoop:
            {
                if (!isGrounded)
                {
                    loco.cur = (velocityY > 0.f) ? LocomotionState::JumpStart : LocomotionState::JumpLoop;
                    PlayKey(loco, (velocityY > 0.f) ? AnimKey::JumpStart : AnimKey::JumpLoop,
                        (velocityY > 0.f) ? ANIMTYPE::ONCE : ANIMTYPE::LOOP, params.fadeShort);
                    break;
                }

                if ((!runInput && speedXZ < params.runStopThreshold) || (speedXZ < params.walkStopThreshold))
                {
                    loco.cur = LocomotionState::RunEnd;
                    PlayKey(loco, AnimKey::RunEnd, ANIMTYPE::ONCE, params.fadeShort);
                }
                break;
            }

            case LocomotionState::RunEnd:
            {
                if (!isGrounded)
                {
                    loco.cur = (velocityY > 0.f) ? LocomotionState::JumpStart : LocomotionState::JumpLoop;
                    PlayKey(loco, (velocityY > 0.f) ? AnimKey::JumpStart : AnimKey::JumpLoop,
                        (velocityY > 0.f) ? ANIMTYPE::ONCE : ANIMTYPE::LOOP, params.fadeShort);
                    break;
                }

                if (IsCurClipFinished(loco))
                {
                    if (speedXZ > params.runStartThreshold && runInput)
                    {
                        loco.cur = LocomotionState::RunStart;
                        PlayKey(loco, AnimKey::RunStart, ANIMTYPE::ONCE, params.fadeVeryShort);
                    }
                    else if (speedXZ > params.walkStartThreshold)
                    {
                        loco.cur = LocomotionState::WalkStart;
                        PlayKey(loco, AnimKey::WalkStart, ANIMTYPE::ONCE, params.fadeVeryShort);
                    }
                    else
                    {
                        loco.cur = LocomotionState::Idle;
                        PlayKey(loco, AnimKey::Idle, ANIMTYPE::LOOP, params.fadeShort);
                    }
                }
                break;
            }

            case LocomotionState::JumpStart:
            {
                if (justLanded)
                {
                    loco.cur = LocomotionState::JumpEnd;
                    PlayKey(loco, AnimKey::JumpEnd, ANIMTYPE::ONCE, params.fadeShort);
                    break;
                }
                if (IsCurClipFinished(loco))
                {
                    loco.cur = LocomotionState::JumpLoop;
                    PlayKey(loco, AnimKey::JumpLoop, ANIMTYPE::LOOP, params.fadeShort);
                }
                break;
            }

            case LocomotionState::JumpLoop:
            {
                if (justLanded)
                {
                    loco.cur = LocomotionState::JumpEnd;
                    PlayKey(loco, AnimKey::JumpEnd, ANIMTYPE::ONCE, params.fadeShort);
                }
                break;
            }

            case LocomotionState::JumpEnd:
            {
                if (IsCurClipFinished(loco))
                {
                    const bool wantRun = (speedXZ > params.runStartThreshold) && runInput;
                    const bool wantWalk = (speedXZ > params.walkStartThreshold) && !wantRun;

                    if (wantRun)
                    {
                        loco.cur = LocomotionState::RunStart;
                        PlayKey(loco, AnimKey::RunStart, ANIMTYPE::ONCE, params.fadeVeryShort);
                    }
                    else if (wantWalk)
                    {
                        loco.cur = LocomotionState::WalkStart;
                        PlayKey(loco, AnimKey::WalkStart, ANIMTYPE::ONCE, params.fadeVeryShort);
                    }
                    else
                    {
                        loco.cur = LocomotionState::Idle;
                        PlayKey(loco, AnimKey::Idle, ANIMTYPE::LOOP, params.fadeShort);
                    }
                }
                break;
            }

            case LocomotionState::FieldSwing:
            {
                if (!IsCurClipFinished(loco)) break;

                if (speedXZ > params.runStartThreshold && runInput)
                {
                    loco.cur = LocomotionState::RunStart;
                    PlayKey(loco, AnimKey::RunStart, ANIMTYPE::ONCE, params.fadeShort);
                }
                else if (speedXZ > params.walkStartThreshold)
                {
                    loco.cur = LocomotionState::WalkStart;
                    PlayKey(loco, AnimKey::WalkStart, ANIMTYPE::ONCE, params.fadeShort);
                }
                else
                {
                    loco.cur = LocomotionState::Idle;
                    PlayKey(loco, AnimKey::Idle, ANIMTYPE::LOOP, params.fadeShort);
                }
                break;
            }
            }

            loco.wasGroundedPrev = isGrounded;
        });
}

void FieldAnimSystem::RenderGui(EntityID id)
{
#ifdef USE_IMGUI
	if (!ImGui::CollapsingHeader("FieldAnimSystem##FieldAnimSystem", ImGuiTreeNodeFlags_DefaultOpen)) return;
	
    GuiUtility::BeginPanel("Field Locomotion", PanelMode::Lines, 10.f);
	ForEachAliveEx([&](Handle handle, EntityID owner, LocomotionAnim& loco)
		{
			if (id != invalidEntity && owner != id) return;

			ImGui::PushID((int)handle.idx);
			if (ImGui::TreeNodeEx("Locomotion", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::Text("Owner: %u", (unsigned)owner);
				ImGui::Text("State: %d", (int)loco.cur);
				ImGui::Text("Clip : %ls", loco.curClipName.c_str());
				ImGui::Text("Elapsed: %.3f", loco.stateElapsed);
				ImGui::TreePop();
			}
			ImGui::PopID();
		});

	GuiUtility::EndPanel();
#endif
}

const wstring& FieldAnimSystem::ResolveClip(const AnimProfile& profile, AnimKey key) const
{
    const auto& animData = registry.Get<AnimDataSystem>();
    return animData.GetClipName(profile.character, profile.context, key);
}

bool FieldAnimSystem::IsCurClipFinished(const LocomotionAnim& loco) const
{
    const auto& animSys = registry.Get<AnimatorSystem>();
    const float duration = animSys.GetClipDuration(loco.animHandle, loco.curClipName);
    if (duration < 0.f) return true;
    return loco.stateElapsed >= duration;
}

void FieldAnimSystem::PlayKey(LocomotionAnim& loco, AnimKey key, ANIMTYPE type, float fadeSec)
{
    auto& animSys = registry.Get<AnimatorSystem>();
    const wstring& clipName = ResolveClip(loco.profile, key);
    if (clipName.empty()) return;

    loco.curClipName = clipName;
    loco.stateElapsed = 0.f;

    if (fadeSec > 0.f)
        animSys.PlayFade(loco.animHandle, loco.layerIdx, clipName, fadeSec, 1.f, type);
    else
        animSys.Play(loco.animHandle, loco.layerIdx, clipName, type);
}
