#include "Enginepch.h"
#include "HierachyPanel.h"

HierachyPanel::HierachyPanel(string title, SystemRegistry& registry, EntityID* selected)
	:GuiPanel(move(title), registry, selected)
{
	layerSys = &registry.Get<LayerSystem>();
	modelSys = &registry.Get<ModelSystem>();
}

void HierachyPanel::Draw()
{
#ifdef USE_IMGUI
	ImGui::InputText("Filter", filter, sizeof(filter));
	ImGui::Separator();

	string loweredFilter = filter;
	transform(loweredFilter.begin(), loweredFilter.end(), loweredFilter.begin(), ::tolower);

	for (_uint i = 0; i < ENUM(LAYER::END); ++i)
	{
		LAYER eLayer = static_cast<LAYER>(i);
		string sLayer = Utility::ToString(eLayer);
		string nodeLabel = sLayer + "##layer_" + to_string(i);

		const _uint mask = LayerUtil::LayerBit(eLayer);

		vector<pair<EntityID, string>> items;
		items.reserve(64);

		layerSys->ForEachByMask(mask, [&](EntityID owner, Handle handle, const LayerData& layer)
			{
				char baseLabel[64];
				sprintf_s(baseLabel, "Entity %u", owner);
				string displayLabel = baseLabel;

				Handle modelHandle{};
				const ModelData* modelComp = modelSys->GetByOwner(owner, &modelHandle);
				if (modelComp && modelComp->model)
				{
					const wstring& logicalKey = modelComp->model->GetLogicalKey();
					if (!logicalKey.empty())
					{
						string str = Utility::StrPathStem(logicalKey);
						if (!str.empty())
						{
							displayLabel += " (";
							displayLabel += str;
							displayLabel += ")";
						}
					}
				}

				string lowered = displayLabel;
				transform(lowered.begin(), lowered.end(), lowered.begin(), ::tolower);
				if (!loweredFilter.empty() && lowered.find(loweredFilter) == string::npos) return;

				items.emplace_back(owner, move(displayLabel));
			});

		if (items.empty()) continue;

		if (ImGui::TreeNode(nodeLabel.c_str()))
		{
			for (const auto& [owner, label] : items)
			{
				const bool isSelected = (selected && *selected == owner);
				if (ImGui::Selectable(label.c_str(), isSelected))
				{
					if (selected) *selected = owner;
				}
			}
			ImGui::TreePop();
		}
	}
#endif
}