#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL FpsPanel final : public GuiPanel
{
public:
	FpsPanel(string title, SystemRegistry& registry, EntityID* selected)
		: GuiPanel(move(title), registry, selected) {
	}

public:
	virtual void Draw() override;
};

NS_END