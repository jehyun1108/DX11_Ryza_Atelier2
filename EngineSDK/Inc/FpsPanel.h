#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL FpsPanel final : public GuiPanel
{
public:
	FpsPanel(string title, SystemRegistry& registry, EntityMgr& entities, EntityID* selected)
		: GuiPanel(move(title), registry, entities, selected) {
	}

public:
	virtual void Draw() override;
};

NS_END