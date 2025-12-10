#pragma once

#include "FieldMinimapData.h"

NS_BEGIN(Engine)

class ENGINE_DLL FieldMinimapPresenter : public ISystem
{
public:
	explicit FieldMinimapPresenter(SystemRegistry& registry) : registry(registry) {}
	void     OnBoot() override;

	void     Enter();
	void     Tick(float dt);
	void     Exit();

	void     SetPlayer(EntityID playerID, Handle playerTf);
	void     SetFieldCam(Handle camTf, Handle cam);

	MinimapScreenRect GetMinimapScreenRect(const wstring& mapKey);

private:
	void     UpdateCam(float dt);
	void     UpdatePlayerCursor();
	void     UpdateEnemyIcons();
	void     UpdateMapScroll();

private:
	EntityID playerID{};
	Handle   playerTf{};
	Handle   fieldCamTf{};
	Handle   fieldCam{};

	wstring minimapFrameKey   = L"field_minimap";      // µÕ±Ù Å×µÎ¸®
	wstring minimapMapKey     = L"field_minimap_in";   // Áß¾Ó¿¡ ºÙÀÏ
	wstring minimapPlayerKey  = L"minimap_cursor";
	wstring minimapNorthKey   = L"north";
	wstring minimapCentralKey = L"minimap_central";
	
	wstring enemyIconRed      = L"minimap_enemy_icon";
	int     enemyIconCount    = 0;

	float camHeight   = 5000.f;
	float camPitchDeg = 90.f; 
	float worldRadius = 2000.f;

private:
	SystemRegistry&  registry;
	UIRegistry*      uiRegistry{};
	UIAnimSystem*    uiAnimSys{};
	UIMinimapSystem* minimapSys{};
	TransformSystem* tfSys{};
	InputService*    input{};
	LayerSystem*     layerSys{};
	UISystem*        uiSys{};
};

NS_END