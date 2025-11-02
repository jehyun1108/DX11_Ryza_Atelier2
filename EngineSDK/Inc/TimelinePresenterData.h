#pragma once

NS_BEGIN(Engine)

struct TimelineTrack
{
    _float2 startPos{ 100.f, 100.f };
    _float2 endPos{ 600.f, 100.f };

    _float2 Dir() const
    {
        _float2 d{ endPos.x - startPos.x, endPos.y - startPos.y };
        const float len = max(0.0001f, sqrtf(d.x * d.x + d.y * d.y));
        return { d.x / len, d.y / len };
    }
    float Length() const
    {
        _float2 d{ endPos.x - startPos.x, endPos.y - startPos.y };
        return sqrtf(d.x * d.x + d.y * d.y);
    }
    // t ∈ [0,1] → 화면 좌표
    _float2 Eval(float t) const
    {
        t = clamp(t, 0.f, 1.f);
        return { startPos.x + (endPos.x - startPos.x) * t,  startPos.y + (endPos.y - startPos.y) * t };
    }
    // 수직 벡터(겹침 해소용)
    _float2 Perp(float scale = 1.f) const
    {
        _float2 dir = Dir();
        return { -dir.y * scale, dir.x * scale };
    }
};

struct TimelineIconRuntime
{
    EntityID   entity   = invalidEntity;
    BattleTeam team     = BattleTeam::Ally;
    bool       isLeader = false;

    float tRaw     = 0.f; //  실시간 값
    float tDisplay = 0.f;

    wstring widgetKey;
    wstring texKey;
    bool    visible    = true;
    int     zOrderBias = 0; // Leader 우선

    _float2 screenPos{};
    float   scale = 1.f; // Leader 더 크게
};

NS_END