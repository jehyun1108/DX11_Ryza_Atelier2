#include "Enginepch.h"

Handle MouthSystem::Create(EntityID owner, Handle anim, wstring clip, _uint layer, float weight, float speed)
{
	Handle handle   = pool.CreateComp(owner);
	auto& mouth     = *pool.Get(handle);
	mouth           = {};
	mouth.animator  = anim;
	mouth.layer     = layer;
	mouth.clip      = move(clip);
	mouth.weight    = weight;
	mouth.speed     = speed;

	auto& animSys = registry.Get<AnimatorSystem>();
	while (layer >= animSys.GetLayerCount(anim))
		animSys.AddLayer(anim, {});
	animSys.SetLayerEnabled(anim, layer, true);

	auto mask = animSys.BuildMaskFromClip(anim, mouth.clip, true);
	animSys.SetLayerMask(anim, layer, mask);
	animSys.SetLayerBlendType(anim, layer, ANIMBLEND::ADDITIVE);
	animSys.SetLayerBlendWeight(anim, layer, mouth.weight);
	animSys.Play(anim, layer, mouth.clip, ANIMTYPE::LOOP);
	animSys.SetPlaybackSpeed(anim, layer, mouth.speed);

	return handle;
}

void MouthSystem::SetWeight(Handle handle, float weight)
{
	if (auto mouth = Get(handle))
	{
		mouth->weight = Utility::Saturate(weight);
		registry.Get<AnimatorSystem>().SetLayerBlendWeight(mouth->animator, mouth->layer, mouth->weight);
	}
}

void MouthSystem::SetSpeed(Handle handle, float speed)
{
	if (auto mouth = Get(handle))
	{
		mouth->speed = (speed < 0.f ? 0.f : speed);
		registry.Get<AnimatorSystem>().SetPlaybackSpeed(mouth->animator, mouth->layer, mouth->speed);
	}
}

void MouthSystem::SetClip(Handle handle, const wstring& clip)
{
	if (auto mouth = Get(handle))
	{
		mouth->clip = clip;
		auto& animSys = registry.Get<AnimatorSystem>();
		auto mask = animSys.BuildMaskFromClip(mouth->animator, mouth->clip, true);
		animSys.SetLayerMask(mouth->animator, mouth->layer, mask);
		animSys.Play(mouth->animator, mouth->layer, mouth->clip, ANIMTYPE::LOOP);
	}
}

void MouthSystem::RenderGui(EntityID id)
{
#ifdef USE_IMGUI
    auto& animSys = registry.Get<AnimatorSystem>();
    ForEachOwned(id, [&](Handle handle, MouthData& mouth)
        {
            ImGui::PushID((int)handle.idx);

            if (ImGui::CollapsingHeader("Mouth"))
            {
                // Read-Only
                ImGui::TextDisabled("Layer: %u (additive)", mouth.layer);

                // Clip select
                {
                    auto names = animSys.GetClipNames(mouth.animator); // vector<wstring>
                    int selected = -1;
                    for (int i = 0; i < (int)names.size(); ++i)
                    {
                        if (names[i] == mouth.clip) 
                        { 
                            selected = i;
                            break;
                        }
                    }

                    const char* cur = (selected >= 0) ? Utility::ToString(names[selected]).c_str() : "(none)";

                    ImGui::SeparatorText("Clip");
                    if (ImGui::BeginCombo("Clip Name", cur))
                    {
                        for (int i = 0; i < (int)names.size(); ++i)
                        {
                            const bool isSelected = (i == selected);
                            const std::string item = Utility::ToString(names[i]);
                            if (ImGui::Selectable(item.c_str(), isSelected))
                            {
                                SetClip(handle, names[i]); // 마스크/재생 갱신
                                selected = i;
                            }
                            if (isSelected)
                                ImGui::SetItemDefaultFocus();
                        }
                        ImGui::EndCombo();
                    }
                }

                // Weight
                {
                    float weight = mouth.weight;
                    if (ImGui::SliderFloat("Weight", &weight, 0.f, 1.f))
                        SetWeight(handle, weight);
                }

                // Speed
                {
                    float speed = mouth.speed;
                    if (ImGui::DragFloat("Speed", &speed, 0.01f, 0.f, 10.f))
                        SetSpeed(handle, speed);
                    ImGui::SameLine();
                    if (ImGui::SmallButton(speed > 0.f ? "Pause" : "Resume"))
                        SetSpeed(handle, (speed > 0.f ? 0.f : 1.f));
                }
            }
            ImGui::PopID();
        });
#endif
}
