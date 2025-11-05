#include "Enginepch.h"
#include "InspectorPanel.h"

InspectorPanel::InspectorPanel(string title, SystemRegistry& registry, EntityID* selected)
	:GuiPanel(move(title), registry, selected)
{
	entities = &registry.Get<EntityMgr>();
}

void InspectorPanel::Draw()
{
#ifdef USE_IMGUI
	if (!selected || *selected == invalidEntity)
	{
		ImGui::TextDisabled("No Entity selected");
		return;
	}

	const EntityID id = *selected;
	bool alive = entities->IsAlive(id);

	ImGui::Separator();
	ImGui::Text("Entity %u %s", id, alive ? "" : "(dead)");
	ImGui::SameLine();

	if (alive && ImGui::SmallButton("Destroy##entity"))
		entities->DestroyDeferred(id);

	ImGui::Separator();

	for (IGuiRenderable* system : registry.GetGuiSystems())
	{
		ImGui::PushID(system);
		system->RenderGui(id);
		ImGui::PopID();
	}
#endif
}
