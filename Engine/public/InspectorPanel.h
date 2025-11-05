#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL InspectorPanel final : public GuiPanel
{
public:
	InspectorPanel(string title, SystemRegistry& registry, EntityID* selected);

public:
	virtual void Draw() override;

private:
	EntityMgr* entities{};
};

NS_END