#include "Enginepch.h"
#include "SoundSystem.h"

void FieldAnimSystem::OnBoot()
{
    animator    = &registry.Get<AnimatorSystem>();
    moveSys     = &registry.Get<MoveStateSystem>();
    intentSys   = &registry.Get<MoveIntentSystem>();
    input       = &registry.Get<InputService>();
    animDataSys = &registry.Get<AnimDataSystem>();
    effectSys   = &registry.Get<EffectSystem>();
    dataSys     = &registry.Get<CharacterDataSystem>();
    tfSys       = &registry.Get<TransformSystem>();
    soundSys    = &registry.Get<SoundSystem>();
}

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

    data.footstepTimer = 0.f;
    data.footstepToggle = false;

    //data.wakeIntroActive = true; 
    //data.wakeIndex = -1;

    animator->SetLayerBlendWeight(animHandle, 0, 1.f);
    animator->SetLayerBlendType(animHandle, 0, ANIMBLEND::OVERRIDE);
    
    PlayKey(data, AnimKey::Idle, ANIMTYPE::LOOP, 0.f);
    return handle;
}

void FieldAnimSystem::Update(float dt)
{
    ForEachAliveEx([&](Handle handle, EntityID owner, LocomotionAnim& loco)
        {
            MoveState* move = moveSys->GetByOwner(owner);
            const MoveIntent* intent = intentSys->GetByOwner(owner);
            if (!move) return;

            const float speedXZ    = sqrtf(move->velocityXZ.x * move->velocityXZ.x + move->velocityXZ.y * move->velocityXZ.y);
            const float velocityY  = move->velocityY;
            const bool  isGrounded = move->grounded;

            const bool  runInput   = intent ? intent->isRunning : (speedXZ >= params.runStartThreshold);

            loco.stateElapsed += dt;

            //if (loco.wakeIntroActive)
            //{
            //    UpdateWakeIntro(loco);
            //    loco.wasGroundedPrev = isGrounded;
            //    return;
            //}

            const bool justLanded     = ( isGrounded && !loco.wasGroundedPrev);
            const bool justLeftGround = (!isGrounded &&  loco.wasGroundedPrev);
            const bool attackEdgeNow = input->ConsumeAttackEdge(owner);

            if (justLeftGround)
            {
                soundSys->PlaySkipDur(L"jump", 0.1f, 0.2f);
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

            if (justLanded)
                soundSys->Play(L"land", 0.2f);

            switch (loco.cur)
            {
            case LocomotionState::Idle:
            {
                if (attackEdgeNow && isGrounded)
                {
                    loco.cur = LocomotionState::FieldSwing;
                    PlayKey(loco, AnimKey::FieldSwing, ANIMTYPE::ONCE, params.fadeShort);
                    effectSys->PlayTrail(L"ryza_trail", dataSys->GetEntityID(CharacterID::Ryza) + 1, 0.25f, 0.3f);
                    //effectSys->PlayTrail(L"fire_trail", dataSys->GetEntityID(CharacterID::Ryza) + 1, 0.25f, 0.3f);
                    soundSys->Play(L"ryza_42");
                    soundSys->Play(L"swing_wand", 0.35f);
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
                    if (speedXZ < params.walkStartThreshold)
                    {
                        loco.cur = LocomotionState::JumpEnd;
                        PlayKey(loco, AnimKey::JumpEnd, ANIMTYPE::ONCE, params.fadeShort);
                    }
                    else if (runInput && speedXZ > params.runStartThreshold)
                    {
                        loco.cur = LocomotionState::RunLoop;
                        PlayKey(loco, AnimKey::RunLoop, ANIMTYPE::LOOP, params.fadeShort);
                    }
                    else
                    {
                        loco.cur = LocomotionState::WalkLoop;
                        PlayKey(loco, AnimKey::WalkLoop, ANIMTYPE::LOOP, params.fadeShort);
                    }
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
                    if (speedXZ < params.walkStartThreshold)
                    {
                        loco.cur = LocomotionState::JumpEnd;
                        PlayKey(loco, AnimKey::JumpEnd, ANIMTYPE::ONCE, params.fadeShort);
                    }
                    else if (runInput && speedXZ > params.runStartThreshold)
                    {
                        loco.cur = LocomotionState::RunLoop;
                        PlayKey(loco, AnimKey::RunLoop, ANIMTYPE::LOOP, params.fadeShort);
                    }
                    else
                    {
                        loco.cur = LocomotionState::WalkLoop;
                        PlayKey(loco, AnimKey::WalkLoop, ANIMTYPE::LOOP, params.fadeShort);
                    }
                    break;
                }

                break;
            }

            case LocomotionState::JumpEnd:
            {
                if (IsCurClipFinished(loco))
                {
                    loco.cur = LocomotionState::Idle;
                    PlayKey(loco, AnimKey::Idle, ANIMTYPE::LOOP, params.fadeShort);
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

            // --------- 발소리 루프 (walk/run loop) ----------
            constexpr float walkStepInterval = 0.80f;
            constexpr float runStepInterval = 0.6f;

            const bool inWalkLoop = (loco.cur == LocomotionState::WalkLoop);
            const bool inRunLoop = (loco.cur == LocomotionState::RunLoop);

            if (isGrounded && (inWalkLoop || inRunLoop) && speedXZ > params.walkStartThreshold)
            {
                float interval = inRunLoop ? runStepInterval : walkStepInterval;

                loco.footstepTimer += dt;
                if (loco.footstepTimer >= interval)
                {
                    loco.footstepTimer -= interval;

                    if (loco.footstepToggle)
                        soundSys->Play(L"ground01_a");
                    else
                        soundSys->Play(L"ground01_b");

                    loco.footstepToggle = !loco.footstepToggle;
                }
            }
            else
            {
                loco.footstepTimer = 0.f;
            }
        });
}

const wstring& FieldAnimSystem::ResolveClip(const AnimProfile& profile, AnimKey key) const
{
    return animDataSys->GetClipName(profile.character, profile.context, key);
}

bool FieldAnimSystem::IsCurClipFinished(const LocomotionAnim& loco) const
{
    const float duration = animator->GetClipDuration(loco.animHandle, loco.curClipName);
    if (duration < 0.f) return true;
    return loco.stateElapsed >= duration;
}

void FieldAnimSystem::PlayKey(LocomotionAnim& loco, AnimKey key, ANIMTYPE type, float fadeSec)
{
    const wstring& clipName = ResolveClip(loco.profile, key);
    assert(!clipName.empty() && "FieldAnimSystem: missing clip for AnimKey in profile (character/context)");

    loco.curClipName = clipName;
    loco.stateElapsed = 0.f;

    const float fadeSeconds = max(0.f, fadeSec);
    if (fadeSeconds > 0.f)
        animator->CrossFade(loco.animHandle, 0, 1, clipName, fadeSeconds, type);
    else
        animator->Play(loco.animHandle, 0, clipName, type);
}

//void FieldAnimSystem::UpdateWakeIntro(LocomotionAnim& loco)
//{
//    if (loco.profile.character != CharacterID::Ryza) return;
//    static constexpr AnimKey seq[] =
//    {
//       AnimKey::WakeUp_A,
//        AnimKey::WakeUp_B,
//        AnimKey::WakeUp_C,
//        AnimKey::WakeUp_D,
//        AnimKey::WakeUp_E,
//        AnimKey::WakeUp_F,
//        AnimKey::WakeUp_G,
//        AnimKey::WakeUp_H,
//        AnimKey::WakeUp_I,
//    };
//    static constexpr int seqCount = sizeof(seq) / sizeof(seq[0]);
//    if (loco.wakeIndex < 0)
//    {
//        loco.wakeIndex = 0;
//        loco.stateElapsed = 0.f;
//        PlayKey(loco, seq[0], ANIMTYPE::ONCE, 0.f);  
//        return;
//    }
//
//    if (!IsCurClipFinished(loco))
//        return;
//
//    loco.wakeIndex++;
//    if (loco.wakeIndex >= seqCount)
//    {
//        loco.wakeIntroActive = false;
//        loco.cur = LocomotionState::Idle;
//        loco.stateElapsed = 0.f;
//        PlayKey(loco, AnimKey::Idle, ANIMTYPE::LOOP, params.fadeShort);
//        return;
//    }
//    loco.stateElapsed = 0.f;
//    PlayKey(loco, seq[loco.wakeIndex], ANIMTYPE::ONCE, params.fadeShort);
//}

void FieldAnimSystem::RenderGui(EntityID id)
{
#ifdef USE_IMGUI
    
    if (ImGui::CollapsingHeader("FieldAnimSystem##FieldAnimSystem"))
    {
        ForEachOwned(id, [&](Handle handle, LocomotionAnim& loco)
            {
                ImGui::PushID((int)handle.idx);

                const ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap;

                if (ImGui::TreeNodeEx("Locomotion", flags))
                {
                    ImGui::Text("Owner: %u", (unsigned)id);
                    ImGui::Text("State: %d", (int)loco.cur);
                    ImGui::Text("Clip : %ls", loco.curClipName.c_str());
                    ImGui::Text("Elapsed: %.3f", loco.stateElapsed);
                    ImGui::TreePop();
                }

                ImGui::PopID();
            });
    }

#endif
}