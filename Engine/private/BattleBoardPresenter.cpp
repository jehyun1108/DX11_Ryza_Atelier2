#include "Enginepch.h"
#include "BattleBoardPresenter.h"
#include "BattleAttributeSystem.h"

void BattleBoardPresenter::OnBoot()
{
    uiReg       = &registry.Get<UIRegistry>();
    uiAnim      = &registry.Get<UIAnimSystem>();
    sessionSys  = &registry.Get<BattleSessionSystem>();
    timelineSys = &registry.Get<BattleTimelineSystem>();
    dataSys     = &registry.Get<CharacterDataSystem>();
    tfSys       = &registry.Get<TransformSystem>();
    attrSys     = &registry.Get<BattleAttributeSystem>(); 

    cfg.boardKey    = L"battle_minimap";        
    cfg.iconBaseKey = L"battle_board_icon_base";

    layout.centerLocal = _float2{ 1050.f, -500.f };
    layout.boardRadius = 120.f;
    layout.maxWorldRadius = 600.f;
}

void BattleBoardPresenter::Enter()
{
    EnsureWidgets();

    uiReg->SetEnabled(cfg.boardKey, true);
    uiReg->SetLocalPos(cfg.boardKey, layout.centerLocal.x, layout.centerLocal.y);

    BuildSlots();
    UpdateIconPositions();
}

void BattleBoardPresenter::Tick(float dt)
{
    UpdateIconPositions();
}

void BattleBoardPresenter::Exit()
{
    for (const auto& slot : slots)
    {
        if (slot.entity == invalidEntity) continue;
        uiReg->SetEnabled(BuildIconKey(slot.entity), false);
    }
    slots.clear();

    uiReg->SetEnabled(cfg.boardKey, false);
}

void BattleBoardPresenter::EnsureWidgets()
{
    uiReg->Ensure(cfg.boardKey);
    uiReg->SetEnabled(cfg.boardKey, false);
    uiReg->Ensure(cfg.iconBaseKey);
    uiReg->SetEnabled(cfg.iconBaseKey, false);
}

void BattleBoardPresenter::BuildSlots()
{
    slots.clear();

    const BattleSessionState& s = sessionSys->GetState();

    auto addSlot = [&](EntityID e, BattleTeam team, int idx)
        {
            if (e == invalidEntity) return;

            BattleBoardSlot slot{};
            slot.entity = e;
            slot.team = team;
            slot.idx = idx;
            slots.push_back(slot);

            const wstring instKey = BuildIconKey(e);
            uiReg->EnsureClone(cfg.iconBaseKey, instKey);

            const wstring texKey = dataSys->GetTextureKey(e, UITextureSlot::TimelineIcon);
            uiReg->SetWidgetTexture(instKey, texKey);
            uiReg->SetEnabled(instKey, true);
        };

    for (int i = 0; i < s.allies.memberCount; ++i)
        addSlot(s.allies.members[i], BattleTeam::Ally, i);

    for (int i = 0; i < s.enemies.memberCount; ++i)
        addSlot(s.enemies.members[i], BattleTeam::Enemy, i);
}

void BattleBoardPresenter::UpdateIconPositions()
{
    for (const auto& slot : slots)
        PlaceIcon(slot);
}

void BattleBoardPresenter::PlaceIcon(const BattleBoardSlot& slot)
{
    const BattleSessionState& s = sessionSys->GetState();
    const _float3 center = s.centerWorld;

    Handle tfHandle{};
    tfSys->GetByOwner(slot.entity, &tfHandle);
    TransformData* tf = tfSys->Get(tfHandle);

    if (!tf)
    {
        const wstring key = BuildIconKey(slot.entity);
        uiReg->SetEnabled(key, false);
        return;
    }

    _float3 d{  tf->pos.x - center.x, 0.f, tf->pos.z - center.z };

    float dist = sqrtf(d.x * d.x + d.z * d.z);
    if (dist < 1e-6f) dist = 1.f;

    float nx = d.x / dist;
    float nz = d.z / dist; 

    float t = dist / layout.maxWorldRadius;
    if (t > 1.f) t = 1.f;

    float r = layout.boardRadius * t;
    float cx = layout.centerLocal.x;
    float cy = layout.centerLocal.y;

    float x = cx + nx * r;
    float y = cy - nz * r; 

    const wstring key = BuildIconKey(slot.entity);
    uiReg->SetLocalPos(key, x, y);

    float hpRatio = attrSys->GetHpRatio01(slot.entity);
    float alpha = (hpRatio > 0.f) ? 1.f : 0.f;
    uiAnim->SetOpacity(key, alpha);

    uiReg->SetZOrder(cfg.boardKey, 500);
    uiReg->SetZOrder(key, 510 + slot.idx);
}