#include "Enginepch.h"

void BattleSessionSystem::OnBoot()
{
    formationSys = &registry.Get<BattleFormationSystem>();
}

void BattleSessionSystem::BeginSession(const BattleParty& allies, const BattleEnemies& enemies, const _float3& centerWorld, const BattleSessionConfig& cfg)
{
    FormationParams intro{};
    FormationParams battle{};

    intro.ringRadius       = 300.f;
    intro.allyStartDeg     = 0.f;
    intro.enemyStartDeg    = 180.f;
    intro.allySpanDeg      = 120.f;
    intro.enemySpanDeg     = 120.f;
    intro.charRadiusMeters = 150.f;
    intro.padDeg           = 4.f;
    intro.backMeters       = 200.f;   

    battle = intro;
    battle.ringRadius = 200.f;      
    battle.backMeters = 400.f;      

    BeginSession(allies, enemies, centerWorld, intro, battle, cfg);
}

void BattleSessionSystem::BeginSession(const BattleParty& allies, const BattleEnemies& enemies, const _float3& centerWorld, const FormationParams& introParams, const FormationParams& battleParams, const BattleSessionConfig& cfg)
{
    sessionState              = BattleSessionState{};
    auto& state               = *sessionState;
    state.phase               = BattlePhase::Intro;
    state.elapsed             = 0.f;
    state.allies              = allies;
    state.enemies             = enemies;
    state.config              = cfg;
    state.victoryCondition    = BattleVictoryCondition{};
    state.isIntroFinished     = false;
    state.isResultFinished    = false;
    state.leaderEntity        = (allies.memberCount > 0) ? allies.members[0] : invalidEntity;
    state.alliesReadyCount    = 0;
    state.introReadyEventSent = false;
    state.alliesReadyEntities.clear();
    state.centerWorld         = centerWorld;

    AssignSlots();
    formationSys->Init(centerWorld, allies.memberCount, enemies.memberCount, introParams, battleParams);
    PushEvent({ BattleSessionEventType::SessionBegan });
}

void BattleSessionSystem::Update(float dt)
{
    if (!sessionState) return;
    auto& state = *sessionState;
    state.elapsed += dt;

    formationSys->Tick(dt);

    switch (state.phase)
    {
    case BattlePhase::Intro:
        if (state.isIntroFinished)
        {
            state.phase = BattlePhase::Active;
            formationSys->SetPhase(FormationPhase::Battle);
            PushEvent({ BattleSessionEventType::SessionActivated });
        }
        break;

    case BattlePhase::Active:
        if (state.isResultFinished)
        {
            state.phase = BattlePhase::Exit;
            PushEvent({ BattleSessionEventType::SessionResultDecided });
        }
        break;

    case BattlePhase::Result:
        break;

    case BattlePhase::Exit:
        break;
    }
}

void BattleSessionSystem::EndSession()
{
    if (!sessionState) return;
    PushEvent({ BattleSessionEventType::SessionEnded });
    sessionState.reset();
    eventQueue.clear();
}

BattleTeam BattleSessionSystem::GetTeam(EntityID entity) const
{
    const auto& s = *sessionState;
    auto it = s.slots.teamByEntity.find(entity);
    return it->second;
}

int BattleSessionSystem::GetSlotIdx(EntityID e) const
{
    const auto& s = *sessionState;

    auto itTeam = s.slots.teamByEntity.find(e);
    if (itTeam->second == BattleTeam::Ally)
    {
        auto it = s.slots.allySlotIdxByEntity.find(e);
        return it->second;
    }
    else
    {
        auto it = s.slots.enemySlotIdxByEntity.find(e);
        return it->second;
    }
}

pair<BattleTeam, int> BattleSessionSystem::GetTeamSlot(EntityID entity) const
{
    const BattleTeam team = GetTeam(entity);
    const int        slot = GetSlotIdx(entity);
    return { team, slot };
}

void BattleSessionSystem::ReportIntroReady(EntityID entity)
{
    auto& state = *sessionState;
    const BattleTeam team = GetTeam(entity);

    const bool already = (state.alliesReadyEntities.find(entity) != state.alliesReadyEntities.end());
    if (!already)
    {
        state.alliesReadyEntities.insert(entity);
        state.alliesReadyCount = min(state.alliesReadyCount + 1, state.allies.memberCount);
    }

    if (!state.introReadyEventSent && state.alliesReadyCount >= state.allies.memberCount)
    {
        state.isIntroFinished = true;
        state.introReadyEventSent = true;
        PushEvent({ BattleSessionEventType::IntroReady });
    }
}

void BattleSessionSystem::ReportResultDecided()
{
    sessionState->isResultFinished = true;
}

void BattleSessionSystem::AssignSlots()
{
    if (!sessionState) return;
    auto& state = *sessionState;

    auto mapAllies = [&](int count)
        {
            int order3[3] = { 1, 0, 2 };
            int order2[2] = { 0, 1 };
            int order1[1] = { 0 };

            int used = 0;
            for (int i = 0; i < count && used < 3; ++i)
            {
                const EntityID e = state.allies.members[i];
                if (e == invalidEntity) continue;

                int slot = 0;
                if (count >= 3) slot = order3[used];
                else if (count == 2) slot = order2[used];
                else slot = order1[used];

                state.slots.allySlotIdxByEntity[e] = slot;
                state.slots.teamByEntity[e] = BattleTeam::Ally;
                ++used;
            }
        };

    auto mapEnemies = [&](int count)
        {
            int order3[3] = { 1, 0, 2 };
            int order2[2] = { 0, 1 };
            int order1[1] = { 0 };

            int used = 0;
            for (int i = 0; i < count && used < 3; ++i)
            {
                const EntityID e = state.enemies.members[i];
                if (e == invalidEntity) continue;

                int slot = 0;
                if (count >= 3) slot = order3[used];
                else if (count == 2) slot = order2[used];
                else slot = order1[used];

                state.slots.enemySlotIdxByEntity[e] = slot;
                state.slots.teamByEntity[e] = BattleTeam::Enemy;
                ++used;
            }
        };

    mapAllies(state.allies.memberCount);
    mapEnemies(state.enemies.memberCount);
}