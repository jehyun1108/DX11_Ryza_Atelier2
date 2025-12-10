#pragma once

#include "BattleTargetHUDData.h"

NS_BEGIN(Engine)

class ENGINE_DLL BattleTargetHUDPresenter : public ISystem
{
public:
    explicit BattleTargetHUDPresenter(SystemRegistry& reg) : registry(reg) {}

    void OnBoot() override;
    void Enter();
    void Tick(float dt);
    void Exit();

private:
    void EnsureWidgets();
    void RefreshLeader();
    void RefreshCurTarget();

    void UpdateHpBars(float dt);
    void UpdateTimelineRings(float dt);
    void UpdateCursorAndLabel(float dt);
    void UpdateSlotVisibility();

    int  FindSlotIdx(EntityID enemy) const;
    int  FindEmptySlot() const;
    
    void AssignSlot(int slotIdx, EntityID enemy);
    void InitEnemySlots();

    void ClearSlot(int slotIdx);
    void ClearAllSlots();

    void EnableSlotWidgets(int slotIdx, bool on);
    void SetSlotHpFront(int slotIdx, float ratio);

    _float2 GetBarPos(int slotIdx) const;
    _float2 GetHpBackPos(int slotIdx) const;
    _float2 GetHpFrontPos(int slotIdx) const;

    _float2 GetIconPos(int slotIdx) const;
    _float2 GetCursorPos(int slotIdx) const;
    _float2 GetLabelPos(int slotIdx) const;
    _float2 GetRingPos(int slotIdx) const;

    void OnLeaderChanged(const BattleEvent& e);

private:
    TargetHUDConfig  cfg{};
    TargetHUDRuntime rt{};

private:
    SystemRegistry&        registry;
    UIRegistry*            uiReg{};
    UIAnimSystem*          uiAnimSys{};
    BattleTargetSystem*    targetSys{};
    BattleTimelineSystem*  timelineSys{};
    CharacterDataSystem*   dataSys{};
    BattleAttributeSystem* attrSys{};
    BattleSessionSystem*   sessionSys{};
};

NS_END