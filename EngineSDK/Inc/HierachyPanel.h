#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL HierachyPanel final : public GuiPanel
{
public:
	HierachyPanel(string title, SystemRegistry& registry, EntityMgr& entities, EntityID* selected) 
		: GuiPanel(move(title), registry, entities, selected) {}

public:
	virtual void Draw() override;

private:
	char filter[256]{};
};

NS_END