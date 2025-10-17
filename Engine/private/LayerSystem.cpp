#include "Enginepch.h"

Handle LayerSystem::Create(EntityID owner, Handle transform, _uint mask)
{
	auto handle     = CreateComp(owner);
	auto& layer     = *Get(handle);
	layer           = {};
	layer.transform = transform;
	layer.layerMask = mask;
	layer.enabled   = true;
	return handle;
}

void LayerSystem::SetMask(Handle handle, _uint mask)
{
	if (auto layer = pool.Get(handle))
		layer->layerMask = mask;
}

void LayerSystem::Enable(Handle handle, bool on)
{
	if (auto layer = pool.Get(handle))
		layer->enabled = on;
}

_uint LayerSystem::GetMask(Handle handle) const
{
	auto layer = pool.Get(handle);
	return layer ? layer->layerMask : 0;
}

void LayerSystem::RenderGui(EntityID id)
{
#ifdef USE_IMGUI
	_uint allBits = 0;
	for (_uint i = 0; i < ENUM(LAYER::END); ++i)
		allBits |= LayerUtil::LayerBit(static_cast<LAYER>(i));

	ForEachOwned(id, [&](Handle handle, LayerData& layer)
		{
            ImGui::PushID((int)handle.idx);

            if (ImGui::CollapsingHeader("Layer"))
            {
                bool enabled = layer.enabled;
                if (ImGui::Checkbox("Enabled", &enabled))
                    Enable(handle, enabled);

                ImGui::SeparatorText("Mask");

                if (ImGui::SmallButton("All"))  
                    SetMask(handle, allBits);

                ImGui::SameLine();
                if (ImGui::SmallButton("None"))
                    SetMask(handle, 0);

                ImGui::Dummy(ImVec2(0, 4));

                _uint curMask = layer.layerMask;
                bool changed = false;

                // 보기 좋게 2열 테이블로 배치
                if (ImGui::BeginTable("layer_mask_tbl", 2, ImGuiTableFlags_SizingStretchProp))
                {
                    for (_uint i = 0; i < ENUM(LAYER::END); ++i)
                    {
                        LAYER eLayer = static_cast<LAYER>(i);
                        string label = Utility::ToString(eLayer);

                        bool checked = LayerUtil::Has(curMask, eLayer);

                        ImGui::TableNextColumn();
                        if (ImGui::Checkbox(label.c_str(), &checked))
                        {
                            changed = true;
                            curMask = checked ? LayerUtil::Add(curMask, eLayer)
                                : LayerUtil::Remove(curMask, eLayer);
                        }
                    }
                    ImGui::EndTable();
                }

                if (changed && curMask != layer.layerMask)
                    SetMask(handle, curMask);

                ImGui::Text("Mask (hex): 0x%08X", curMask);

                ImGui::SeparatorText("Info");
                ImGui::Text("Owner    : %u", GetOwner(handle));
                ImGui::Text("Transform: idx=%u gen=%u %s",
                    layer.transform.idx, layer.transform.generation,
                    layer.transform.IsValid() ? "" : "(none)");
            }

            ImGui::PopID();
		});
#endif
}
