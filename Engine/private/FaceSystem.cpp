#include "Enginepch.h"

Handle FaceSystem::Create(EntityID owner, Handle anim, wstring openClip, wstring closeClip, float openDur, float openJitter, float holdClose, float fadeClose, float fadeOpen)
{
    Handle handle   = CreateComp(owner);
    auto& face      = *Get(handle);
    face            = {};
    face.animator   = anim;
    face.layer      = 1;
    face.clipOpen   = move(openClip);
    face.clipClose  = move(closeClip);
    face.openDur    = openDur;
    face.openJitter = openJitter;
    face.holdClose  = holdClose;
    face.fadeClose  = fadeClose;
    face.fadeOpen   = fadeOpen;

    auto& animSys = registry.Get<AnimatorSystem>();
    while(face.layer >= animSys.GetLayerCount(anim))
        animSys.AddLayer(anim, {});
    animSys.SetLayerEnabled(anim, face.layer, true);

    auto a = animSys.BuildMaskFromClip(anim, face.clipOpen, true);
    auto b = animSys.BuildMaskFromClip(anim, face.clipClose, true);
    vector<uint8_t> mask(max(a.size(), b.size()), 0);
    for (size_t i = 0; i < mask.size(); ++i)
    {
        uint8_t maskA = (i < a.size()) ? a[i] : 0;
        uint8_t maskB = (i < b.size()) ? b[i] : 0;
        mask[i] = uint8_t(maskA | maskB);
    }
    animSys.SetLayerMask(anim, face.layer, mask);

    animSys.Play(anim, face.layer, face.clipClose, ANIMTYPE::LOOP);
    float closePoseTime = animSys.GetClipDuration(anim, face.clipClose);
    animSys.SetLayerTime(anim, face.layer, closePoseTime);
    animSys.SetPlaybackSpeed(anim, face.layer, 0.f);
    animSys.SetLayerBlendType(anim, face.layer, ANIMBLEND::OVERRIDE);
    animSys.SetLayerBlendWeight(anim, face.layer, 0.f);

    face.curOpenHold = nextOpenHold(face);
    face.curFadeClose = Utility::Jitter(face.fadeClose);
    face.curFadeOpen = Utility::Jitter(face.fadeOpen);
    face.state = FaceState::OpenHold;
    face.timer = face.curOpenHold;
    return handle;
}

void FaceSystem::SetSpeed(Handle handle, float speed)
{
    if (auto face = Get(handle))
        face->speedScale = max(0.1f, speed);
}

void FaceSystem::Update(float dt)
{
    auto& animSys = registry.Get<AnimatorSystem>();
    ForEachAlive([&](_uint, FaceData& face)
        {
            const float step = dt * face.speedScale;

            switch (face.state)
            {
            case FaceState::OpenHold:
                face.timer -= step;
                if (face.timer <= 0.f)
                {
                    face.curFadeClose = Utility::Jitter(face.fadeClose);
                    face.curFadeOpen = Utility::Jitter(face.fadeOpen);
                    face.state = FaceState::Closing;
                    face.timer = face.fadeClose;
                }
                break;

            case FaceState::Closing:
            {
                face.timer = max(0.f, face.timer - step);
                const float t = 1.f - Utility::Saturate(face.timer / max(1e-5f, face.fadeClose));
                const float weight = Utility::Saturate(Utility
                    ::EaseCosine(t));
                animSys.SetLayerBlendWeight(face.animator, face.layer, weight);

                if (face.timer <= 0.f)
                {
                    animSys.SetLayerBlendWeight(face.animator, face.layer, 1.f);
                    if (face.holdClose <= 0.f)
                    {
                        face.state = FaceState::Opening;
                        face.timer = face.curFadeOpen;
                    }
                    else
                    {
                        face.state = FaceState::CloseHold;
                        face.timer = max(0.f, face.holdClose * (0.8f + 0.4f * Utility::Range(0.f, 1.f)));
                    }
                }
                break;
            }

            case FaceState::CloseHold:
                face.timer -= step;
                animSys.SetLayerBlendWeight(face.animator, face.layer, 1.f);
                if (face.timer <= 0.f)
                {
                    face.state = FaceState::Opening;
                    face.timer = face.fadeOpen;
                }
                break;

            case FaceState::Opening:
            {
                face.timer = max(0.f, face.timer - step);
                const float t = 1.f - Utility::Saturate(face.timer / max(1e-5f, face.fadeOpen));
                const float weight = Utility::Saturate(1.f - Utility::EaseCosine(t));
                animSys.SetLayerBlendWeight(face.animator, face.layer, weight);

                if (face.timer <= 0.f)
                {
                    animSys.SetLayerBlendWeight(face.animator, face.layer, 0.f);
                    face.curOpenHold = nextOpenHold(face);
                    face.state = FaceState::OpenHold;
                    face.timer = face.curOpenHold;
                }
                break;
            }
            }
        });
}

float FaceSystem::nextOpenHold(const FaceData& face)
{
    const float low = max(0.25f, face.openDur - face.openJitter);
    const float high = face.openDur + face.openJitter;
    return low + (high - low) * Utility::Range(0.f, 1.f);
}

void FaceSystem::RenderGui(EntityID id)
{
#ifdef USE_IMGUI
    ForEachOwned(id, [&](Handle handle, FaceData& face)
        {
            ImGui::PushID((int)handle.idx);

            if (ImGui::CollapsingHeader("Face (Blink)"))
            {
                ImGui::TextDisabled("Layer: %u (override)", face.layer);

                ImGui::SeparatorText("Clips");
                ImGui::Text("Open  : %s", Utility::ToString(face.clipOpen).c_str());
                ImGui::Text("Close : %s", Utility::ToString(face.clipClose).c_str());

                ImGui::SeparatorText("Timing");
                bool changed = false;

                changed |= ImGui::DragFloat("Open Duration", &face.openDur, 0.01f, 0.01f, 10.f, "%.3f");
                changed |= ImGui::DragFloat("Open Jitter", &face.openJitter, 0.01f, 0.f, 10.f, "%.3f");
                changed |= ImGui::DragFloat("Hold Close", &face.holdClose, 0.01f, 0.f, 1.0f, "%.3f");
                changed |= ImGui::DragFloat("Fade Close", &face.fadeClose, 0.001f, 0.001f, 1.0f, "%.3f");
                changed |= ImGui::DragFloat("Fade Open", &face.fadeOpen, 0.001f, 0.001f, 1.0f, "%.3f");

                ImGui::SeparatorText("Speed Scale");
                ImGui::DragFloat("Speed", &face.speedScale, 0.01f, 0.1f, 5.f, "%.2f");

                ImGui::Separator();
                // 즉시 깜박임 트리거
                if (ImGui::SmallButton("Blink Now"))
                {
                    face.curFadeClose = Utility::Jitter(face.fadeClose);
                    face.curFadeOpen = Utility::Jitter(face.fadeOpen);
                    face.state = FaceState::Closing;
                    face.timer = face.fadeClose; 
                }
                ImGui::SameLine();
                if (ImGui::SmallButton("Reset Cycle"))
                {
                    face.curOpenHold = nextOpenHold(face);
                    face.curFadeClose = Utility::Jitter(face.fadeClose);
                    face.curFadeOpen = Utility::Jitter(face.fadeOpen);
                    face.state = FaceState::OpenHold;
                    face.timer = face.curOpenHold;
                }

                // 상태 표시(읽기전용)
                ImGui::SeparatorText("State");
                const char* state =
                    (face.state == FaceState::OpenHold) ? "OpenHold" :
                    (face.state == FaceState::Closing) ? "Closing" :
                    (face.state == FaceState::CloseHold) ? "CloseHold" :
                    "Opening";
                ImGui::Text("State : %s", state);
                ImGui::Text("Timer : %.3f", face.timer);
                ImGui::Text("curFadeClose : %.3f | curFadeOpen : %.3f | nextOpenHold : %.3f",
                    face.curFadeClose, face.curFadeOpen, face.curOpenHold);
            }
            ImGui::PopID();
        });
#endif
}