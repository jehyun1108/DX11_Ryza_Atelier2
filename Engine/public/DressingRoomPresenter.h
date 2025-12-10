#pragma once

#include "DressingRoomData.h"

NS_BEGIN(Engine)

class ENGINE_DLL DressingRoomPresenter : public ISystem
{
public:
	explicit DressingRoomPresenter(SystemRegistry& registry) : registry(registry) {}
	void     BuildInitData();      

	void OnBoot() override;
	void Enter();
	void Tick(float dt);
	void Exit();

	bool IsActive() const { return state != DressingRoomState::Hidden; }

	void BuildDressingCamera(CameraProxy& outCam);
	void BuildDressingDrawItems(vector<DrawItem>& outItems);

private:
	void SetUpUIInstances();    
	void RefreshTabs();    
	void RefreshTabHighlight();
	void RefreshCostumeList(); 

	void SetCostumePanelVisible(bool visible);
	void HandleTabInput();
	void HandleCostumeInput();

	void EquipSelected();
	void ApplyCostume(const CharacterDressingData& ch, const CostumeDef& def);

private:
	DressingRoomState             state = DressingRoomState::Hidden;
	DressingFocus                 focus = DressingFocus::CharacterTabs;
	int                           activeCharIdx = 0;
	DressingRoomUIConfig          uiConfig;
	vector<CharacterDressingData> characters;

	float highlightOffsetX = 0.f;
	float highlightOffsetY = 0.f;
	bool  highlightOffsetInit = false;

	float tabHighlightOffsetX    = 0.f;
	float tabHighlightOffsetY    = 0.f;
	bool  tabHighlightOffsetInit = false;
	float dressingOrbitAngle     = XM_PI;
	float dressingZoom           = 2.f;
	float targetDressingZoom     = 2.0f;

	float dressingPanRight = 0.f; // 카메라 좌/우 운동량
	float dressingPanUp    = 0.f; // 카메라 위/아래 운동량

	float prevOrbitAngle = XM_PI;
	float orbitAccumulatedRad = 0.f;
	bool  ryzaZoomSfxPlayed = false;

private:
	SystemRegistry&      registry;
	UISystem*            uiSys{};
	UIRegistry*          uiReg{};
	UIAnimSystem*        uiAnim{};
	CharacterDataSystem* dataSys{};
	InputService*        input{};
	TransformSystem*     tfSys{};
	ModelSystem*         modelSys{};
	AnimatorSystem*      animator{};
	AssetSystem*         assets{};
	AnimDataSystem*      animDataSys{};
	SoundSystem*         soundSys{};
};

NS_END