#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL CharacterUIAgent
{
public:
	CharacterUIAgent(UIRegistry& uiRegistry, UIAnimSystem& uiAnimSys) 
		: uiRegistry(uiRegistry), uiAnimSys(uiAnimSys){}

	void EnsureAll();
	void ShowBaseHUD();
	void HideBaseHUD();
	void HideAll();
	void PlaceBaseHUD(int slotIdx, float startX, float startY, float verticalSpacing);

private:
	UIRegistry& uiRegistry;
	UIAnimSystem& uiAnimSys;
};

NS_END