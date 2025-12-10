#pragma once

NS_BEGIN(Engine)

struct Spot
{
    wstring id;          // "ShrineDragon" 이런 내부 id
    wstring name;        // "신령한 용의 관" UI 텍스트
    _float2 iconPos;     // worldmap_icon 위치
    _float2 clickPos;    // worldmap_clickicon 위치
    _float2 barPos;      // barback 위치 (텍스트도 여기 근처)
    _float2 labelOffset;
    _float3 targetWorld = { 0.f, 0.f, 0.f }; // 플레이어 텔레포트할 월드 좌표
    float   clickRadius = 120.f;
    wstring bgmKey;
};
struct MapViewState
{
    _float2 center = { 0.f, 0.f }; 
    float   scale  = 1.f;         

    _float2 startCenter = { 0.f, 0.f };
    float   startScale  = 1.f;

    _float2 targetCenter = { 0.f, 0.f };
    float   targetScale  = 1.f;

    float   t       = 1.f;  
    float   dur     = 0.5f;  
    bool    playing = false;

    float minScale = 1.f;
    float maxScale = 2.f;
};
struct ConfirmUI
{
    bool  active      = false;
    int   selectedIdx = 0;
    float fadeDur     = 0.25f;

    wstring barKey       = L"field_selectbar";
    wstring highlightKey = L"field_select_highlight";
    wstring yesKey       = L"worldmap_select_yes";
    wstring noKey        = L"worldmap_select_no";

    _float2 highlightYesPos = { 800.f, 110.f }; 
    _float2 highlightNoPos  = { 800.f, 170.f };
};

NS_END