#include "Enginepch.h"
#include "WorldMapPresenter.h"
#include "SoundSystem.h"

void WorldMapPresenter::OnBoot()
{
    uiRegistry = &registry.Get<UIRegistry>();
    uiSys      = &registry.Get<UISystem>();
    uiAnimSys  = &registry.Get<UIAnimSystem>();
    tfSys      = &registry.Get<TransformSystem>();
    input      = &registry.Get<InputService>();
    soundSys   = &registry.Get<SoundSystem>();
}

void WorldMapPresenter::Init()
{
    spots.clear();
    spots.reserve(10);

    const float offsetY = 75.f;
    spots.push_back(Spot{ L"ShrineDragon", L"신령한 용의 관", { -550.f, 40.f }, { -550.f, 75.f }, { -550.f, 150.f }, {0.f, 30.f}, { 15000.f, 8000.f, -13000.f }, 120.f, L"Valley_BGM"});
    spots.push_back(Spot{ L"SuzuCastle", L"수저 성도", { -850.f, -225.f }, { -855.f, -200.f }, { -855.f, -125.f }, { 0.f, 30.f }, { 0.f, 0.f, 0.f } });
    spots.push_back(Spot{ L"DragonRidge", L"전승의 용골 협곡", { -400.f, -500.f }, { -425.f, -475.f }, { -425.f, -400.f }, { 0.f, 30.f }, { 0.f, 0.f, 0.f }, 120.f, L"Valley_BGM" });
    spots.push_back(Spot{ L"AncientManaAtelier", L"고대 마나 공방", { -200.f, 500.f }, { -200.f, 475.f }, { -200.f, 550.f }, { 0.f, 30.f }, {0.f, 0.f, 0.f} });
    spots.push_back(Spot{ L"PhantomLand", L"환상의 땅", { 700.f, -475.f }, { 690.f, -450.f }, { 690.f, -375.f }, { 0.f, 30.f }, {42000.f, 300.f, 362.f}, 120.f});
    spots.push_back(Spot{ L"UndergroundGrave", L"지하소녀의 묘지", { 400.f, -200.f }, { 375.f, -150.f }, { 375.f, -75.f }, { 0.f, 30.f }, {0.f,0.f,0.f}});
    spots.push_back(Spot{ L"CapitalAsraAmBert", L"왕도 아슬라 암 버트", {-9999.f, -9999.f}, { -965.f, 190.f }, { -975.f, 265.f }, { 0.f, 30.f }, {820.f, 370.f, -2528.f}, 120.f, L"Central_BGM" });
    spots.push_back(Spot{ L"CapitalSouth", L"왕도 남쪽", {-9999.f, -9999.f}, { -950.f, 450.f }, { -950.f, 525.f }, { 0.f, 30.f }, {42000.f,100.f,300.f}});
    spots.push_back(Spot{ L"CapitalOutskirts", L"라이자의 아틀리에", { -9999.f, -9999.f }, { -50.f, 75.f }, { -50.f, 150.f }, { 0.f, 30.f }, {33000.f, 50.f, 3500.f}, 120.f, L"House_BGM" });
    spots.push_back(Spot{ L"WindyValley", L"바람이 부는 골짜기", { -9999.f, -9999.f }, { 650.f, 25.f }, { 650.f, 100.f }, {0.f, 30.f}, {0.f,0.f,0.f}});

    worldmapUIKeys.clear();
    {
        UIInstance& bg = uiRegistry->Ensure(L"worldmap");
        bg.selfEnabled = false;
        worldmapUIKeys.push_back(L"worldmap");
    }

    const auto& archetypes = uiRegistry->GetArchetypes();

    for (size_t i = 0; i < spots.size(); ++i)
    {
        const Spot& spot = spots[i];

        if (spot.iconPos.x > -9000.f)
        {
            wstring iconKey = L"worldmap_icon_" + to_wstring(i);

            auto itArc = archetypes.find(iconKey);
            if (itArc != archetypes.end())
            {
                UIInstance& icon = uiRegistry->Ensure(iconKey);
                icon.selfEnabled = false;
                worldmapUIKeys.push_back(iconKey);
            }
        }
        {
            wstring clickKey = L"worldmap_clickicon_" + to_wstring(i);
            UIInstance& click = uiRegistry->Ensure(clickKey);
            click.selfEnabled = false;
            worldmapUIKeys.push_back(clickKey);
        }
        {
            wstring barKey = L"worldmap_barback_" + to_wstring(i);
            UIInstance& bar = uiRegistry->Ensure(barKey);
            bar.selfEnabled = false;
            worldmapUIKeys.push_back(barKey);
        }
    }
    {
        UIInstance& back = uiRegistry->Ensure(dialogue.characterBackKey);
        back.selfEnabled = false;
        UIInstance& icon = uiRegistry->Ensure(dialogue.characterIconKey);
        icon.selfEnabled = false;
        UIInstance& dlg  = uiRegistry->Ensure(dialogue.dialogueBackKey);
        dlg.selfEnabled  = false;
        UIInstance& text = uiRegistry->Ensure(dialogue.introTextKey);
        text.selfEnabled = false;
    }
    {
        UIInstance& cursor = uiRegistry->Ensure(selectKey);
        cursor.selfEnabled = false;
        worldmapUIKeys.push_back(selectKey);
    }

    dialogue.state = IntroDialogue::State::Hidden;
    dialogue.timer = 0.f;
    selectAngleRad = 0.f;

    mapView.center       = _float2{ 0.f, 0.f };
    mapView.scale        = 1.f;
    mapView.startCenter  = mapView.center;
    mapView.startScale   = mapView.scale;
    mapView.targetCenter = mapView.center;
    mapView.targetScale  = mapView.scale;
    mapView.t            = 1.f;
    mapView.dur          = 0.5f;
    mapView.playing      = false;
}

void WorldMapPresenter::Enter()
{
    isActive = true;
    for (const wstring& key : worldmapUIKeys)
        uiRegistry->SetEnabled(key, true);

    uiRegistry->SetEnabled(selectKey, true);
    cursorVisible = true;
    uiAnimSys->SetOpacity(selectKey, 1.f);

    {
        UIInstance& bar = uiRegistry->Ensure(confirm.barKey);
        bar.selfEnabled = false;

        UIInstance& hl = uiRegistry->Ensure(confirm.highlightKey);
        hl.selfEnabled = false;

        UIInstance& yesInst = uiRegistry->Ensure(confirm.yesKey);
        yesInst.selfEnabled = false;

        UIInstance& noInst = uiRegistry->Ensure(confirm.noKey);
        noInst.selfEnabled = false;
    }

    confirm.active = false;
    confirm.selectedIdx = 0;

    state = MapState::Idle;
    curSpotIdx = -1;
    lastTeleportSpotIdx = -1;

    for (size_t i = 0; i < spots.size(); ++i)
    {
        const Spot& s = spots[i];

        wstring instKey   = L"worldmap_label_" + to_wstring(i);
        UIInstance& label = uiRegistry->EnsureClone(L"worldmap_label", instKey);
        label.selfEnabled = true;
        label.text        = s.name;      
    }

    const auto& vp = GAME.GetViewport();
    float screenW = static_cast<float>(vp.Width);
    float screenH = static_cast<float>(vp.Height);

    auto [mapTexW, mapTexH] = uiRegistry->GetOrCacheTexSize(L"worldmap");

    mapHalfSize = _float2{ mapTexW * 0.5f, mapTexH * 0.5f };

    float sx = screenW / mapTexW;
    float sy = screenH / mapTexH;

    mapView.minScale = max(sx, sy);           
    mapView.maxScale = mapView.minScale * 2.f; 

    mapView.center       = _float2{ 0.f, 0.f }; 
    mapView.scale        = mapView.minScale;     
    mapView.startCenter  = mapView.center;
    mapView.startScale   = mapView.scale;
    mapView.targetCenter = mapView.center;
    mapView.targetScale  = mapView.scale;
    mapView.t            = 1.f;
    mapView.playing      = false;

    if (!spotsNormalized)
    {
        const float inv = 1.f / mapView.minScale;

        for (auto& s : spots)
        {
            s.iconPos.x     *= inv;
            s.iconPos.y     *= inv;
            s.clickPos.x    *= inv;
            s.clickPos.y    *= inv;
            s.barPos.x      *= inv;
            s.barPos.y      *= inv;
            s.labelOffset.x *= inv;
            s.labelOffset.y *= inv;
            s.clickRadius   *= inv; 
        }
        spotsNormalized = true;
    }
    static bool firstEnter = true;
    if (!firstEnter)
    {
        soundSys->Play(L"enter_worldmap");
        soundSys->Play(L"patricia_36");
        soundSys->PlayAfter(L"patricia_28", 2.f);
    }
    firstEnter = false;

    StartIntroDialogue(L"어디에 가실 건가요? 주인님");
    RebuildLayout();
}

void WorldMapPresenter::Tick(float dt)
{
    _float2 mouse = input->GetMousePos();

    const auto& vp = GAME.GetViewport();
    float screenW = static_cast<float>(vp.Width);
    float screenH = static_cast<float>(vp.Height);
    float cx = screenW * 0.5f;
    float cy = screenH * 0.5f;

    float localX = mouse.x - cx;
    float localY = mouse.y - cy;

    uiRegistry->SetLocalPos(selectKey, localX, localY);
    uiAnimSys->Spin(selectKey, 120.f);

    if (state == MapState::Idle)
    {
        if (input->KeyDown(KEY::LBUTTON))
        {
            int hitIdx = HitTestSpot(localX, localY);
            if (hitIdx >= 0)
            {
                const Spot& spot = spots[hitIdx];
                StartZoomToSpot(spot, 1.8f, hitIdx);
            }
        }
    }
    if (input->KeyDown(KEY::RBUTTON))
    {
        if (state == MapState::ZoomingToSpot || state == MapState::Confirming)
        {
            HideConfirmPanel();
            StartZoomToBase();
        }
    }

    UpdateMapView(dt);
    RebuildLayout();
    UpdateIntroDialogue(dt);

    if (state == MapState::Confirming)
        UpdateConfirmInput();
}

void WorldMapPresenter::Exit()
{
    static bool firstExit = true;
    if (!firstExit)
        soundSys->Play(L"worldmap_transfer");
    firstExit = false;

    isActive = false;
    for (const wstring& key : worldmapUIKeys)
        uiRegistry->SetEnabled(key, false);

    uiRegistry->SetEnabled(selectKey, false);

    for (size_t i = 0; i < spots.size(); ++i)
    {
        wstring instKey = L"worldmap_label_" + to_wstring(i);
        uiRegistry->SetEnabled(instKey, false);
    }
    uiRegistry->SetEnabled(dialogue.characterBackKey, false);
    uiRegistry->SetEnabled(dialogue.characterIconKey, false);
    uiRegistry->SetEnabled(dialogue.dialogueBackKey,  false);
    uiRegistry->SetEnabled(dialogue.introTextKey,     false);
    dialogue.state = IntroDialogue::State::Hidden;
    dialogue.timer = 0.f;
    HideConfirmPanelImmediate();
}

int WorldMapPresenter::HitTestSpot(float localX, float localY) const
{
    _float2 screenLocal{ localX, localY };
    _float2 mapPos = ScreenToMap(screenLocal);  

    for (size_t i = 0; i < spots.size(); ++i)
    {
        const Spot& s = spots[i];

        float dx = mapPos.x - s.clickPos.x;
        float dy = mapPos.y - s.clickPos.y;
        float dist2 = dx * dx + dy * dy;
        float r = s.clickRadius;   
        float r2 = r * r;

        if (dist2 <= r2)
            return static_cast<int>(i);
    }
    return -1;
}

void WorldMapPresenter::StartIntroDialogue(const wstring& text)
{
    dialogue.msg = text;

    uiRegistry->SetEnabled(dialogue.characterBackKey, true);
    uiRegistry->SetEnabled(dialogue.characterIconKey, true);
    uiRegistry->SetEnabled(dialogue.dialogueBackKey,  true);
    uiRegistry->SetEnabled(dialogue.introTextKey,     true);

    uiSys->SetText(dialogue.introTextKey, dialogue.msg);

    const float dx = -400.f;          
    const float dur = dialogue.slideInDur;

    uiAnimSys->PlaySlideOnce(dialogue.characterBackKey, dx, 0.f, 0.f, 0.f, dur);
    uiAnimSys->PlaySlideOnce(dialogue.characterIconKey, dx, 0.f, 0.f, 0.f, dur);
    uiAnimSys->PlaySlideOnce(dialogue.dialogueBackKey,  dx, 0.f, 0.f, 0.f, dur);
    uiAnimSys->PlaySlideOnce(dialogue.introTextKey,     dx, 0.f, 0.f, 0.f, dur);

    dialogue.state = IntroDialogue::State::SlidingIn;
    dialogue.timer = 0.f;
}

void WorldMapPresenter::StartIntroSlideOut()
{
    const float dx = -400.f;          
    const float dur = dialogue.slideOutDur;

    uiAnimSys->PlaySlideOnce(dialogue.characterBackKey, 0.f, 0.f, dx, 0.f, dur);
    uiAnimSys->PlaySlideOnce(dialogue.characterIconKey, 0.f, 0.f, dx, 0.f, dur);
    uiAnimSys->PlaySlideOnce(dialogue.dialogueBackKey,  0.f, 0.f, dx, 0.f, dur);
    uiAnimSys->PlaySlideOnce(dialogue.introTextKey,     0.f, 0.f, dx, 0.f, dur);

    dialogue.state = IntroDialogue::State::SlidingOut;
    dialogue.timer = 0.f;
}

void WorldMapPresenter::UpdateIntroDialogue(float dt)
{
    using State = IntroDialogue::State;

    if (dialogue.state == State::Hidden) return;

    dialogue.timer += dt;

    switch (dialogue.state)
    {
    case State::SlidingIn:
        if (dialogue.timer >= dialogue.slideInDur)
        {
            dialogue.state = State::Hold;
            dialogue.timer = 0.f;
        }
        break;

    case State::Hold:
        if (dialogue.timer >= dialogue.holdDur)
            StartIntroSlideOut();
        break;

    case State::SlidingOut:
        if (dialogue.timer >= dialogue.slideOutDur)
        {
            dialogue.state = State::Hidden;
            dialogue.timer = 0.f;

            uiRegistry->SetEnabled(dialogue.characterBackKey, false);
            uiRegistry->SetEnabled(dialogue.characterIconKey, false);
            uiRegistry->SetEnabled(dialogue.dialogueBackKey,  false);
            uiRegistry->SetEnabled(dialogue.introTextKey,     false);
        }
        break;

    default:
        break;
    }
}

void WorldMapPresenter::StartZoomToSpot(const Spot& spot, float zoomScale, int spotIdx)
{
    mapView.startCenter = mapView.center;
    mapView.startScale = mapView.scale;
    mapView.targetCenter = spot.clickPos;
    mapView.targetScale = clamp(zoomScale, mapView.minScale, mapView.maxScale);

    mapView.t = 0.f;
    mapView.dur = 0.6f;
    mapView.playing = true;

    curSpotIdx = spotIdx;
    state = MapState::ZoomingToSpot;

    HideCursorForZoom();
    HideConfirmPanelImmediate();
}

void WorldMapPresenter::UpdateMapView(float dt)
{
    if (!mapView.playing) return;

    float denom = max(mapView.dur, 1e-5f);
    mapView.t += dt / denom;

    float tNorm = (mapView.t >= 1.f) ? 1.f : mapView.t;
    float eased = Utility::EaseCubic(tNorm);

    mapView.center.x = lerp(mapView.startCenter.x, mapView.targetCenter.x, eased);
    mapView.center.y = lerp(mapView.startCenter.y, mapView.targetCenter.y, eased);
    mapView.scale = lerp(mapView.startScale, mapView.targetScale, eased);

    mapView.scale = clamp(mapView.scale, mapView.minScale, mapView.maxScale);
    ClampMapView();

    if (tNorm >= 1.f)
    {
        mapView.t = 1.f;
        mapView.playing = false;

        if (state == MapState::ZoomingToSpot)
        {
            state = MapState::Confirming;
            ShowConfirmPanel();
        }
        else if (state == MapState::ZoomingBack)
        {
            state = MapState::Idle;
            ShowCursorAfterZoom();
            HideConfirmPanelImmediate();
        }
    }
}

void WorldMapPresenter::RebuildLayout()
{
    const auto& vp = GAME.GetViewport();
    float screenW = static_cast<float>(vp.Width);
    float screenH = static_cast<float>(vp.Height);
    float cx = screenW * 0.5f;
    float cy = screenH * 0.5f;

    {
        _float2 mapCenterPos{ 0.f, 0.f };          
        _float2 scr = MapToScreen(mapCenterPos);   

        uiRegistry->SetLocalPos(L"worldmap", scr.x, scr.y);
        uiRegistry->SetScale(L"worldmap", mapView.scale, mapView.scale);
    }
    for (size_t i = 0; i < spots.size(); ++i)
    {
        const Spot& s = spots[i];
        if (s.iconPos.x > -5000.f)
        { // icon
            wstring iconKey = L"worldmap_icon_" + to_wstring(i);
            _float2 p = MapToScreen(s.iconPos);
            uiRegistry->SetLocalPos(iconKey, p.x, p.y);
            uiRegistry->SetScale(iconKey, mapView.scale, mapView.scale);
        }
        { // click icon
            wstring clickKey = L"worldmap_clickicon_" + to_wstring(i);
            _float2 p = MapToScreen(s.clickPos);
            uiRegistry->SetLocalPos(clickKey, p.x, p.y);
            uiRegistry->SetScale(clickKey, mapView.scale, mapView.scale);
        }
        { // barback
            wstring barKey = L"worldmap_barback_" + to_wstring(i);
            _float2 p = MapToScreen(s.barPos);
            uiRegistry->SetLocalPos(barKey, p.x, p.y);
            uiRegistry->SetScale(barKey, mapView.scale, mapView.scale);
        }  
        { // label (텍스트)
            wstring labelKey = L"worldmap_label_" + to_wstring(i);

            _float2 mapLabelPos;
            mapLabelPos.x = s.barPos.x + s.labelOffset.x;
            mapLabelPos.y = s.barPos.y + s.labelOffset.y;

            _float2 p = MapToScreen(mapLabelPos);
            uiRegistry->SetLocalPos(labelKey, p.x, p.y);

            uiRegistry->SetScale(labelKey, mapView.scale, mapView.scale);
        }
    }
}

_float2 WorldMapPresenter::MapToScreen(const _float2& mapPos) const
{
    _float2 r{};
    r.x = (mapPos.x - mapView.center.x) * mapView.scale;
    r.y = (mapPos.y - mapView.center.y) * mapView.scale;
    return r;
}

_float2 WorldMapPresenter::ScreenToMap(const _float2& screenLocal) const
{
    assert(mapView.scale > 0.f);
    _float2 r{};
    r.x = screenLocal.x / mapView.scale + mapView.center.x;
    r.y = screenLocal.y / mapView.scale + mapView.center.y;
    return r;
}

void WorldMapPresenter::ClampMapView()
{
    const auto& vp    = GAME.GetViewport();
    float screenW     = static_cast<float>(vp.Width);
    float screenH     = static_cast<float>(vp.Height);
    float halfScreenW = screenW * 0.5f;
    float halfScreenH = screenH * 0.5f;

    float s = mapView.scale;
    assert(s > 0.f);

    if (s < mapView.minScale) return;

    float minCenterX = -mapHalfSize.x + halfScreenW / s;
    float maxCenterX =  mapHalfSize.x - halfScreenW / s;
    float minCenterY = -mapHalfSize.y + halfScreenH / s;
    float maxCenterY =  mapHalfSize.y - halfScreenH / s;

    mapView.center.x = clamp(mapView.center.x, minCenterX, maxCenterX);
    mapView.center.y = clamp(mapView.center.y, minCenterY, maxCenterY);
}

void WorldMapPresenter::StartZoomToBase()
{
    mapView.startCenter = mapView.center;
    mapView.startScale = mapView.scale;
    mapView.targetCenter = _float2{ 0.f, 0.f };
    mapView.targetScale = mapView.minScale;

    mapView.t = 0.f;
    mapView.dur = 0.6f;
    mapView.playing = true;

    state = MapState::ZoomingBack;

    HideCursorForZoom();
}

void WorldMapPresenter::HideCursorForZoom()
{
    if (!cursorVisible) return;

    if (nextHideCursorIsRyza)
        soundSys->Play(L"ryza_57");
    else
        soundSys->Play(L"klaudia_43");

    nextHideCursorIsRyza = !nextHideCursorIsRyza;

    uiAnimSys->PlayFadeOnce(selectKey, 1.f, 0.f, cursorFadeDur);
    cursorVisible = false;
}

void WorldMapPresenter::ShowCursorAfterZoom()
{
    if (cursorVisible) return;
    uiRegistry->SetEnabled(selectKey, true);         
    uiAnimSys->PlayFadeOnce(selectKey, 0.f, 1.f, cursorFadeDur); 
    cursorVisible = true;
}

void WorldMapPresenter::ShowConfirmPanel()
{
    confirm.active = true;
    confirm.selectedIdx = 0;

    uiRegistry->SetEnabled(confirm.barKey, true);
    uiRegistry->SetEnabled(confirm.highlightKey, true);
    uiRegistry->SetEnabled(confirm.yesKey, true);
    uiRegistry->SetEnabled(confirm.noKey, true);

    float d = confirm.fadeDur;

    uiAnimSys->PlayFadeOnce(confirm.barKey, 0.f, 1.f, d);
    uiAnimSys->PlayFadeOnce(confirm.highlightKey, 0.f, 1.f, d);

    const float selA = 1.f;
    const float unSelA = 0.5f;
    uiAnimSys->PlayFadeOnce(confirm.yesKey, 0.f, selA, d);
    uiAnimSys->PlayFadeOnce(confirm.noKey, 0.f, unSelA, d);

    _float2 pos = confirm.highlightYesPos;
    uiRegistry->SetLocalPos(confirm.highlightKey, pos.x, pos.y);
}

void WorldMapPresenter::HideConfirmPanel()
{
    if (!confirm.active) return;
    confirm.active = false;

    float d = confirm.fadeDur;
    uiAnimSys->PlayFadeOnce(confirm.barKey, 1.f, 0.f, d);
    uiAnimSys->PlayFadeOnce(confirm.highlightKey, 1.f, 0.f, d);
    uiAnimSys->PlayFadeOnce(confirm.yesKey, 1.f, 0.f, d);
    uiAnimSys->PlayFadeOnce(confirm.noKey,  1.f, 0.f, d);
}

void WorldMapPresenter::HideConfirmPanelImmediate()
{
    confirm.active = false;

    uiRegistry->SetEnabled(confirm.barKey,       false);
    uiRegistry->SetEnabled(confirm.highlightKey, false);
    uiRegistry->SetEnabled(confirm.yesKey,       false);
    uiRegistry->SetEnabled(confirm.noKey,        false);
}

void WorldMapPresenter::UpdateConfirmHighlight()
{
    const float selA = 1.f;
    const float unSelA = 0.5f;

    _float2 pos = (confirm.selectedIdx == 0)  ? confirm.highlightYesPos : confirm.highlightNoPos;

    uiRegistry->SetLocalPos(confirm.highlightKey, pos.x, pos.y);

    float switchDur = 0.1f;

    if (confirm.selectedIdx == 0)
    {
        uiAnimSys->FadeTo(confirm.yesKey, selA, switchDur);
        uiAnimSys->FadeTo(confirm.noKey, unSelA, switchDur);
    }
    else
    {
        uiAnimSys->FadeTo(confirm.yesKey, unSelA, switchDur);
        uiAnimSys->FadeTo(confirm.noKey, selA, switchDur);
    }
}

void WorldMapPresenter::UpdateConfirmInput()
{
    if (!confirm.active) return;

    if (input->KeyDown(KEY::UP) || input->KeyDown(KEY::DOWN))
    {
        confirm.selectedIdx = 1 - confirm.selectedIdx;
        UpdateConfirmHighlight();
    }

    if (input->KeyDown(KEY::ENTER))
    {
        if (confirm.selectedIdx == 0)
            TeleportToSpot(curSpotIdx);

        HideConfirmPanel();
        StartZoomToBase();
    }

    if (input->KeyDown(KEY::ESC))
    {
        HideConfirmPanel();
        StartZoomToBase();
    }
}

void WorldMapPresenter::TeleportToSpot(int spotIdx)
{
    if (spotIdx < 0 || spotIdx >= static_cast<int>(spots.size())) return;

    const Spot& s = spots[spotIdx];
    const _float3& dst = s.targetWorld;

    lastTeleportSpotIdx = spotIdx;
    pendingTeleportPos = dst;
    hasPendingTeleport = true;

    if (!s.bgmKey.empty())
        soundSys->PlayBgm(s.bgmKey);
}

const wstring& WorldMapPresenter::GetLastTeleportSpotName() const
{
    assert(lastTeleportSpotIdx >= 0 && lastTeleportSpotIdx < static_cast<int>(spots.size()));
    return spots[static_cast<size_t>(lastTeleportSpotIdx)].name;
}

_float3 WorldMapPresenter::ConsumeTeleport()
{
    hasPendingTeleport = false;
    return pendingTeleportPos;
}