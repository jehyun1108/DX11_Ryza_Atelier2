#include "Enginepch.h"
#include "BattleAttributeSystem.h"

static inline void UpdateStunFlags(StunState& s, const BattleAttributeConfig& cfg)
{
    if (!s.full && s.cur >= s.max)
    {
        s.cur = min(s.cur, s.max);
        s.full = true;
        if (cfg.clearOnFull) { s.cur = 0.f; s.full = false; }
    }
    else if (s.full && s.cur < s.max)
        s.full = false;
}
//======================================================================================
void BattleAttributeSystem::OnBoot()
{
	dataSys    = &registry.Get<CharacterDataSystem>();
    eventBus   = &registry.Get<BattleEventBus>();
    sessionSys = &registry.Get<BattleSessionSystem>();
}

void BattleAttributeSystem::InitForSession(const BattleParty& allies, const BattleEnemies& enemies)
{
	hpByEntity.clear();
	stunByEntity.clear();

    auto initSide = [&](const BattleSide& side)
        {
            for (int i = 0; i < side.memberCount; ++i)
            {
                const EntityID entity = side.members[i];
                const CharacterID id = dataSys->GetCharacterID(entity);
                const CharacterSpec& spec = dataSys->GetSpec(id);
                const int maxHp = (spec.team == BattleTeam::Ally) ? spec.party.baseMaxHp : spec.enemy.baseMapHp;
                hpByEntity[entity] = HPState{ maxHp, maxHp };
                stunByEntity[entity] = StunState{ 0.f, config.stunMax, false };
            }
        };

    initSide(allies);
    initSide(enemies);
}

void BattleAttributeSystem::EndSession()
{
    hpByEntity.clear();
    stunByEntity.clear();
}

void BattleAttributeSystem::Tick(float dt)
{
    if (config.stunDecayPerSec > 0.f)
    {
        for (auto& kv : stunByEntity)
        {
            StunState& state = kv.second;
            state.cur = max(0.f, state.cur - config.stunDecayPerSec * dt);
        }
    }

    for (auto& kv : stunByEntity)
    {
        StunState& state = kv.second;
        if (!state.full && state.cur >= state.max)
        {
            state.full = true;
            if (config.clearOnFull)
            {
                state.cur = 0.f;
                state.full = false;
            }
        }
        if (state.full && state.cur < state.max)
            state.full = false;
    }
}

float BattleAttributeSystem::GetHpRatio01(EntityID entity) const
{
    const auto& hp = RequireHp(entity);
    return (hp.max > 0) ? Utility::Saturate(static_cast<float>(hp.cur) / static_cast<float>(hp.max)) : 0.f;
}

float BattleAttributeSystem::GetStunRatio01(EntityID entity) const
{
    const auto& stun = RequireStun(entity);
    return (stun.max > 0.f) ? Utility::Saturate(stun.cur / stun.max) : 0.f;
}

void BattleAttributeSystem::ApplyDamage(EntityID entity, int dmg)
{
    HPState& hp = RequireHp(entity);

    const int prevHp = hp.cur;
    hp.cur = max(0, hp.cur - dmg);

    if (prevHp > 0 && hp.cur == 0)
    {
        BattleEvent e{};
        e.eventType = BattleBusEventType::UnitDowned;
        e.subjectEntity = entity;
        e.subjectTeam = sessionSys->GetTeam(entity); 

        eventBus->Publish(e);
    }
}

void BattleAttributeSystem::Heal(EntityID entity, int amount)
{
    auto& hp = RequireHp(entity);
    hp.cur = min(hp.max, hp.cur + amount);
}

void BattleAttributeSystem::SetMaxHp(EntityID entity, int maxHp)
{
    auto& hp = RequireHp(entity);
    hp.max = maxHp;
    if (hp.cur > hp.max) 
        hp.cur = hp.max;
}

void BattleAttributeSystem::SetStun(EntityID entity, const StunState& inState)
{
    StunState& state = RequireStun(entity);
    state = inState;
    state.max = config.stunMax;
    UpdateStunFlags(state, config);
}

void BattleAttributeSystem::AddStun(EntityID entity, float amount)
{
    StunState& state = RequireStun(entity);
    state.max = config.stunMax;
    state.cur = max(0.f, state.cur + amount);
    UpdateStunFlags(state, config);
}

void BattleAttributeSystem::ApplyHit(EntityID target, int dmg, float stun)
{
    if (dmg != 0) 
        ApplyDamage(target, dmg);
    if (stun != 0) 
        AddStun(target, stun);
}

void BattleAttributeSystem::Reserve(size_t n)
{
    hpByEntity.reserve(n);
    stunByEntity.reserve(n);
}

HPState& BattleAttributeSystem::RequireHp(EntityID entity)
{
    auto it = hpByEntity.find(entity);
    return it->second;
}

const HPState& BattleAttributeSystem::RequireHp(EntityID entity) const
{
    auto it = hpByEntity.find(entity);
    return it->second;
}

StunState& BattleAttributeSystem::RequireStun(EntityID entity)
{
    auto it = stunByEntity.find(entity);
    return it->second;
}

const StunState& BattleAttributeSystem::RequireStun(EntityID entity) const
{
    auto it = stunByEntity.find(entity);
    return it->second;
}

void BattleAttributeSystem::InitSide(const BattleSide& side)
{
    for (int i = 0; i < side.memberCount; ++i)
    {
        const EntityID entity = side.members[i];
        const CharacterID id = dataSys->GetCharacterID(entity);
        const CharacterSpec& spec = dataSys->GetSpec(id);
        const int maxHp = (spec.team == BattleTeam::Ally) ? spec.party.baseMaxHp : spec.enemy.baseMapHp;

        hpByEntity[entity] = HPState{ maxHp, maxHp };
        stunByEntity[entity] = StunState{ 0.f, config.stunMax, false };
    }
}