#include "Enginepch.h"
#include "HierachyPanel.h"

void HierachyPanel::Draw()
{
#ifdef USE_IMGUI
	ImGui::InputText("Filter", filter, sizeof(filter));
	ImGui::Separator();

	string sFilter = filter;
	transform(sFilter.begin(), sFilter.end(), sFilter.begin(), ::tolower);

	auto& layerSys = registry.Get<LayerSystem>();

	for (_uint i = 0; i < ENUM(LAYER::END); ++i)
	{
		LAYER eLayer = static_cast<LAYER>(i);
		string sLayer = Utility::ToString(eLayer);
		string nodeLabel = sLayer + "##layer_" + to_string(i);

		const _uint mask = LayerUtil::LayerBit(eLayer);

		vector<pair<EntityID, string>> items;
		layerSys.ForEachByMask(mask, [&](EntityID owner, Handle handle, const LayerData& layer)
			{
				char label[64];
				sprintf_s(label, "Entity %u", owner);

				string lowered = label;
				transform(lowered.begin(), lowered.end(), lowered.begin(), ::tolower);
				if (!sFilter.empty() && lowered.find(sFilter) == string::npos)
					return;

				items.emplace_back(owner, string(label));
			});

		if (items.empty())
			continue;

		if (ImGui::TreeNode(nodeLabel.c_str()))
		{
			for (const auto& [owner, label] : items)
			{
				const bool isSelected = (selected && *selected == owner);
				if (ImGui::Selectable(label.c_str(), isSelected))
				{
					if (selected)
						*selected = owner;
				}
			}
			ImGui::TreePop();
		}
	}
#endif
}