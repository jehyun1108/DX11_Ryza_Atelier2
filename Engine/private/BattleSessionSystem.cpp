#include "Enginepch.h"

void BattleSessionSystem::OnBoot()
{
    formationSys = &registry.Get<BattleFormationSystem>();
    assert(formationSys);
}

void BattleSessionSystem::BeginSession(const BattleParty& allies, const BattleEnemies& enemies, const _float3& centerWorld, const BattleSessionConfig& cfg)
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

    AssignSlots();
    FormationParams formationParams{};
    formationSys->Init(centerWorld, allies.memberCount, enemies.memberCount, formationParams);
    PushEvent({ BattleSessionEventType::SessionBegan });
}

void BattleSessionSystem::BeginSession(const BattleParty& allies, const BattleEnemies& enemies, const _float3& centerWorld, const FormationParams& formationParams, const BattleSessionConfig& cfg)
{
    sessionState               = BattleSessionState{};
    auto& state                = *sessionState;
    state.phase                = BattlePhase::Intro;
    state.elapsed              = 0.f;
    state.allies               = allies;
    state.enemies              = enemies;
    state.config               = cfg;
    state.victoryCondition     = BattleVictoryCondition{};
    state.isIntroFinished      = false;
    state.isResultFinished     = false;
    state.leaderEntity         = (allies.memberCount > 0) ? allies.members[0] : invalidEntity;
    state.alliesReadyCount     = 0;
    state.introReadyEventSent  = false;
    state.alliesReadyEntities.clear();

    AssignSlots();
    formationSys->Init(centerWorld, allies.memberCount, enemies.memberCount, formationParams);
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

bool BattleSessionSystem::TryGetTeam(EntityID entity, BattleTeam& out) const
{
    if (!sessionState) return false;
    const auto& state = *sessionState;
    auto it = state.slots.teamByEntity.find(entity);
    if (it == state.slots.teamByEntity.end()) return false;
    out = it->second;
    return true;
}

bool BattleSessionSystem::TryGetSlotIdx(EntityID e, int& out) const
{
    if (!sessionState) return false;
    const auto& state = *sessionState;

    auto itTeam = state.slots.teamByEntity.find(e);
    if (itTeam == state.slots.teamByEntity.end()) return false;

    if (itTeam->second == BattleTeam::Ally)
    {
        auto it = state.slots.allySlotIdxByEntity.find(e);
        if (it == state.slots.allySlotIdxByEntity.end()) return false;
        out = it->second; return true;
    }
    else if (itTeam->second == BattleTeam::Enemy)
    {
        auto it = state.slots.enemySlotIdxByEntity.find(e);
        if (it == state.slots.enemySlotIdxByEntity.end()) return false;
        out = it->second; return true;
    }
    return false;
}

void BattleSessionSystem::ReportIntroReady(EntityID entity)
{
    if (!sessionState || entity == invalidEntity) return;
    auto& state = *sessionState;

    BattleTeam team;
    if (!TryGetTeam(entity, team)) return;

    if (team == BattleTeam::Ally)
    {
        if (state.alliesReadyEntities.find(entity) != state.alliesReadyEntities.end()) return;

        state.alliesReadyEntities.insert(entity);
        state.alliesReadyCount = min(state.alliesReadyCount + 1, state.allies.memberCount);

        if (!state.introReadyEventSent && state.alliesReadyCount >= state.allies.memberCount)
        {
            state.isIntroFinished     = true;
            state.introReadyEventSent = true;
            PushEvent({ BattleSessionEventType::IntroReady });
        }
    }
}

void BattleSessionSystem::ReportResultDecided()
{
    if (!sessionState) return;
    sessionState->isResultFinished = true;
}

void BattleSessionSystem::AssignSlots()
{
    if (!sessionState) return;
    auto& state = *sessionState;

    state.slots.allySlotIdxByEntity.clear();
    state.slots.enemySlotIdxByEntity.clear();
    state.slots.teamByEntity.clear();

    int allyUsed = 0;
    for (int i = 0; i < state.allies.memberCount && allyUsed < 3; ++i)
    {
        EntityID entity = state.allies.members[i];
        if (entity == invalidEntity) continue;
        state.slots.allySlotIdxByEntity[entity] = allyUsed;
        state.slots.teamByEntity[entity] = BattleTeam::Ally;
        ++allyUsed;
    }

    int enemyUsed = 0;
    for (int i = 0; i < state.enemies.memberCount && enemyUsed < 3; ++i)
    {
        EntityID entity = state.enemies.members[i];
        if (entity == invalidEntity) continue;
        state.slots.enemySlotIdxByEntity[entity] = enemyUsed;
        state.slots.teamByEntity[entity] = BattleTeam::Enemy;
        ++enemyUsed;
    }
}