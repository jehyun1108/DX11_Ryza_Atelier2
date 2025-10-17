#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL InspectorPanel final : public GuiPanel
{
public:
	InspectorPanel(string title, SystemRegistry& registry, EntityMgr& entities, EntityID* selected)
		: GuiPanel(move(title), registry, entities, selected) {}

public:
	virtual void Draw() override;
};

NS_END