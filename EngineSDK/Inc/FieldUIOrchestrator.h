#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL FieldUIOrchestrator : public ISystem
{
public:
    explicit FieldUIOrchestrator(SystemRegistry& registry) : registry(registry) {}
    void     OnBoot() override;

    void Enter();
    void Tick(float dt);
    void Exit();
    void ShowMapTitle(const wstring& name);

private:
    enum class WorldMapPhase
    {
        Idle, FadingOpen, Showing, FadingClose
    };
    enum class MapTitlePhase
    {
        Hidden, Waiting, Showing
    };
    enum class DressingPhase
    {
        Hidden, FadingOpen, Showing, FadingClose
    };

    bool           worldMapOpen  = false;
    WorldMapPhase  worldPhase    = WorldMapPhase::Idle;

    bool           dressingOpen  = false;
    DressingPhase  dressingPhase = DressingPhase::Hidden;

    wstring        mapTitleBarKey     = L"maptitle_barback";
    wstring        mapTitleSpinnerKey = L"maptitle_spinner";
    wstring        mapTitleTextKey    = L"maptitle_text";
   
    MapTitlePhase  mapTitlePhase  = MapTitlePhase::Hidden;
    wstring        mapTitlePendingName;
    float          mapTitleDelay  = 1.2f;
    float          mapTitleTimer  = 0.f;
    float          mapTitleDur    = 4.f;

private:
    SystemRegistry&        registry;
    UIRegistry*            uiRegistry{};
    UISystem*              uiSys{};
    UIAnimSystem*          uiAnimSys{};
    FieldMinimapPresenter* fieldMini{};
    WorldMapPresenter*     worldPresenter{};
    DressingRoomPresenter* dressingPresenter{};
    InputService*          input{};
    ScreenFadeSystem*      fadeSys{};
};

NS_END