#include "Enginepch.h"

void BattleTacticSystem::OnBoot()
{
	eventBus = &registry.Get<BattleEventBus>();
	Reset();
}

void BattleTacticSystem::Reset()
{
	state.level     = 1;
	state.pips      = 0;
	state.maxBlink  = false;
	Emit(TacticEventType::Changed);
}

void BattleTacticSystem::GainPip(int n)
{
    if (state.level < 5)
    {
        state.pips += n;
        while (state.pips >= 5 && state.level < 5)
        {
            state.pips -= 5;
            ++state.level;
            Emit(TacticEventType::LevelUp);
        }
        state.pips = min(state.pips, 5);
        state.maxBlink = false;
        Emit(TacticEventType::Changed);
    }
    else
    {
        state.pips = min(state.pips + n, 5);
        if (state.pips == 5)
        {
            if (!state.maxBlink)
            { 
                state.maxBlink = true;
                Emit(TacticEventType::MaxBlinkOn);
            }
        }
        else
            if (state.maxBlink)
            {
                state.maxBlink = false;
                Emit(TacticEventType::MaxBlinkOff);
            }
        Emit(TacticEventType::Changed);
    }
}

void BattleTacticSystem::Emit(TacticEventType type)
{
    BattleEvent e{};
    e.subjectEntity = 0;
    e.subjectTeam = BattleTeam::Neutral;

    switch (type)
    {
    case TacticEventType::Changed:    e.eventType = BattleBusEventType::TacticChanged;    break;
    case TacticEventType::LevelUp:    e.eventType = BattleBusEventType::TacticLevelUp;    break;
    case TacticEventType::MaxBlinkOn: e.eventType = BattleBusEventType::TacticMaxBlinkOn; break;
    case TacticEventType::MaxBlinkOff:e.eventType = BattleBusEventType::TacticMaxBlinkOff; break;
    }

    EventPayload_Tactic p{};
    p.level    = state.level;
    p.pips     = state.pips;
    p.maxBlink = state.maxBlink;
    p.type     = type;
    e.payload  = p;

    eventBus->Publish(e);
}