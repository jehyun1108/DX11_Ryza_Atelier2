#pragma once

#include "WorldMapData.h"

NS_BEGIN(Engine)

class ENGINE_DLL WorldMapPresenter : public ISystem
{
public:
	explicit WorldMapPresenter(SystemRegistry& registry) : registry(registry) {}
    void     OnBoot() override;
    void     Init();

    void Enter();
    void Tick(float dt);
    void Exit();

    void  TeleportToSpot(int spotIdx);
    const wstring& GetLastTeleportSpotName() const;
    bool  HasLastTeleport() const { return lastTeleportSpotIdx >= 0; }
    bool  HasTeleport()     const { return hasPendingTeleport; }
    _float3 ConsumeTeleport();
    bool IsActive() const { return isActive; }

private:
    int  HitTestSpot(float localX, float localY) const;
    void StartIntroDialogue(const wstring& text);
    void StartIntroSlideOut();
    void UpdateIntroDialogue(float dt);

    void StartZoomToSpot(const Spot& spot, float zoomScale, int spotIdx);
    void UpdateMapView(float dt);
    void RebuildLayout();

    _float2  MapToScreen(const _float2& mapPos) const;
    _float2  ScreenToMap(const _float2& screenLocal) const;
    void     ClampMapView();
    void     StartZoomToBase();
    
    void HideCursorForZoom();
    void ShowCursorAfterZoom();

    void ShowConfirmPanel();
    void HideConfirmPanel();
    void HideConfirmPanelImmediate();
    void UpdateConfirmHighlight();
    void UpdateConfirmInput();

private:
    struct IntroDialogue
    {
        enum class State
        {
            Hidden, SlidingIn, Hold, SlidingOut
        };

        wstring characterBackKey = L"worldmap_character_back";
        wstring characterIconKey = L"worldmap_patricia";
        wstring dialogueBackKey  = L"worldmap_dialogue";
        wstring introTextKey     = L"worldmap_introtext";

        State state       = State::Hidden;
        float timer       = 0.f;
        float slideInDur  = 1.2f;
        float holdDur     = 4.f;
        float slideOutDur = 1.2f;

        wstring msg;
    };
    enum class MapState
    {
        Idle, ZoomingToSpot, ZoomingBack, Confirming
    };
    bool isActive = false;
    MapViewState    mapView;
    _float2         mapHalfSize = { 0.f, 0.f };
    bool            spotsNormalized = false;

    IntroDialogue   dialogue;
    vector<Spot>    spots;
    vector<wstring> worldmapUIKeys;

    wstring selectKey    = L"worldmap_select";
    float selectAngleRad = 0.f;
    float selectSpeedRad = XMConvertToRadians(120.f);

    bool  cursorVisible = true;
    float cursorFadeDur = 0.5f;

    MapState  state = MapState::Idle;
    ConfirmUI confirm{};
    int       curSpotIdx = -1;
    int       lastTeleportSpotIdx = -1;

    _float3 pendingTeleportPos{};
    bool    hasPendingTeleport = false;
    bool    nextHideCursorIsRyza = false;

private:
    SystemRegistry&         registry;
    UIRegistry*             uiRegistry{};
    UISystem*               uiSys{};
    UIAnimSystem*           uiAnimSys{};
    TransformSystem*        tfSys{};
    InputService*           input{};
    SoundSystem*            soundSys{};
};

NS_END