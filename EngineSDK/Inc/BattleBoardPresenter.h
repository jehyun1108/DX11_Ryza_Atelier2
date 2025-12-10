#pragma once

#include "BattleBoardData.h"

NS_BEGIN(Engine)

class ENGINE_DLL BattleBoardPresenter : public ISystem
{
public:
	explicit BattleBoardPresenter(SystemRegistry& registry) : registry(registry) {}
    void     OnBoot() override;

    void Enter();
    void Tick(float dt);
    void Exit();

private:
    void EnsureWidgets();
    void BuildSlots();
    void UpdateIconPositions();

    void    PlaceIcon(const BattleBoardSlot& slot);
    wstring BuildIconKey(EntityID entity) const { return L"battle_board_icon_" + to_wstring(static_cast<_ulong>(entity)); }

private:
    BattleBoardConfig      cfg{};
    BattleBoardLayout      layout{};

    vector<BattleBoardSlot> slots;

private:
    SystemRegistry&        registry;
    UIRegistry*            uiReg{};
    UIAnimSystem*          uiAnim{};
    BattleSessionSystem*   sessionSys{};
    BattleTimelineSystem*  timelineSys{};
    CharacterDataSystem*   dataSys{};
    TransformSystem*       tfSys{};
    BattleAttributeSystem* attrSys{};
 
};

NS_END