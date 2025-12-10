#pragma once

NS_BEGIN(Engine)

struct PlayerInputSlotKeys
{
    wstring back;
    wstring outline;
    wstring icon;
    wstring key;
    wstring label;
};
struct PlayerInputViewKeys
{
    array<PlayerInputSlotKeys, 4> slots{
        PlayerInputSlotKeys{ L"input_barback1", L"input_outline1", L"defend_icon",   L"input_y",  L"input_label1" },
        PlayerInputSlotKeys{ L"input_barback2", L"input_outline2", L"attack_icon",   L"input_b",  L"input_label2" },
        PlayerInputSlotKeys{ L"input_barback3", L"input_outline3", L"itemrush_icon", L"input_x",  L"input_label3" },
        PlayerInputSlotKeys{ L"input_barback4", L"input_outline4", L"flee_icon",     L"input_a",  L"input_label4" },
    };
};
struct PlayerInputAnimCfg
{
    float dimA         = 0.2f;
};
struct PlayerInputPresenterConfig
{
    PlayerInputViewKeys keys{};
    PlayerInputAnimCfg  anim{};

    array<_float2, 4> labelPos{
        _float2{ 1050.f,  380.f },   // slot0 : 방어
        _float2{ 1575.f,  380.f },   // slot1 : 공격
        _float2{ 1050.f,  455.f },   // slot2 : 아이템
        _float2{ 1575.f,  455.f },   // slot3 : 도망
    };
    array<wstring, 4> highlightKeys{
        L"input_highlight_circle_yellow",
        L"input_highlight_circle_red",
        L"input_highlight_circle_blue",
        L"input_highlight_circle_green",
    };
};
struct PlayerInputPresenterRuntime
{
    EntityID leader = invalidEntity;
    int      curAp = 0;
    int      maxAp = 0;
    bool     ready = false;
    bool     anyKeyHeld = false;
    bool     primaryEverReady = false;
    array<float, 4> highlightT{};
};

struct SkillSlotView
{
    wstring iconTexKey;
    wstring labelText;
};
struct SkillViewSet
{
    array<SkillSlotView, 4> slots;
};

NS_END