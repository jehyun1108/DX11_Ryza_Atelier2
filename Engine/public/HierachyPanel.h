#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL HierachyPanel final : public GuiPanel
{
public:
	HierachyPanel(string title, SystemRegistry& registry, EntityID* selected);

public:
	virtual void Draw() override;

private:
	char filter[256]{};

	LayerSystem* layerSys{};
	ModelSystem* modelSys{};
};

NS_END