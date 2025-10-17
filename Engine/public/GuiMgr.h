#pragma once

#include "ImGuiPrettyTheme.h"

NS_BEGIN(Engine)
class GuiPanel;

class ENGINE_DLL GuiMgr
{
public:
	GuiMgr(SystemRegistry& registry, EntityMgr& entities) :registry(registry), entities(entities) {}
	virtual ~GuiMgr();
	GuiMgr(const GuiMgr& other) = delete;
	GuiMgr& operator=(const GuiMgr&) = delete;

public:
	static unique_ptr<GuiMgr> Create(SystemRegistry& registry, EntityMgr& entities);

	void Init();
	void ShutDown();

	void Update(float dt);
	void Render();

	LRESULT ImguiWndProcHandler(_uint msg, WPARAM wParam, LPARAM lParam);

	template<typename T, typename...Args>
	T* AddPanel(string title, Args&&... args);

	EntityID GetSelected() const { return selected; }
	void     SetSelected(EntityID id) { selected = id; }

private:
	vector<unique_ptr<GuiPanel>> panels;
	SystemRegistry& registry;
	EntityMgr&      entities;
	EntityID        selected = invalidEntity;
};

template<typename T, typename ...Args>
inline T* GuiMgr::AddPanel(string title, Args&& ...args)
{
	auto panel = make_unique<T>(move(title), registry, entities, &selected, forward<Args>(args)...);
	T* pPanel = panel.get();
	panels.emplace_back(move(panel));
	return pPanel;
}

NS_END