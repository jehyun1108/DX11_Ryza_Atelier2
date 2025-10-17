#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL GuiPanel 
{
public:
	GuiPanel(string title, SystemRegistry& registry, EntityMgr& entities, EntityID* selected)
		:title(move(title)), registry(registry), entities(entities), selected(selected) {}
	virtual ~GuiPanel() = default;

public:
	virtual void Update(float dt) {}
	void DrawPanel();

protected:
	virtual void Draw() = 0;

protected:
	GameInstance& game = GameInstance::GetInstance();
	string          title;
	bool            isActive = true;
	SystemRegistry& registry;
	EntityMgr&      entities;
	EntityID*       selected;
};

NS_END