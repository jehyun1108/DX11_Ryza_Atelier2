#include "Enginepch.h"

static double g_controllerTimeSec = 0.0;
static inline int FindIndexIn(const array<KEY, 4>& keys, KEY k)
{
    for (int i = 0; i < 4; ++i) if (keys[(size_t)i] == k) return i;
    return -1;
}
// ------------------------------------------------------------------------------------------------------------------------------
void BattleControllerSystem::Update(EntityID leaderEntity, float dt)
{
    g_controllerTimeSec += static_cast<double>(dt);

    auto& input = registry.Get<InputService>();
    runtime.leaderEntity = leaderEntity;

    if (leaderEntity == invalidEntity)
    {
        runtime.menu.page = CommandMenuPage::Hidden;
        runtime.isDefendingHold = false;
        runtime.isEscapeHolding = false;
        return;
    }

    const bool isReady = IsUnitReadyToAct(runtime.leaderEntity);

    const bool defendHolding = input.KeyPressing(KEY::Y);
    if (defendHolding && !runtime.isDefendingHold)
    {
        runtime.isDefendingHold = true;
        registry.Get<BattleTimelineSystem>().SetUnitGate(runtime.leaderEntity, TimelineUnitGate::Closed);
    }
    if (!defendHolding && runtime.isDefendingHold)
    {
        runtime.isDefendingHold = false;
        registry.Get<BattleTimelineSystem>().SetUnitGate(runtime.leaderEntity, TimelineUnitGate::Open);
    }

    const bool isSkillPageHeld = input.KeyPressing(KEY::SPACE);
    runtime.menu.page = isSkillPageHeld ? CommandMenuPage::Skill : CommandMenuPage::Primary;

    if (runtime.menu.page == CommandMenuPage::Skill)
    {
        if (!isReady) return;
        auto& timelineSys = registry.Get<BattleTimelineSystem>();

        auto tryCommitSkill = [&](KEY key)
            {
                if (!input.KeyDown(key)) return;
                const int idx = FindIndexIn(config.keymap.skill.skillKeys, key);
                if (idx < 0) return;
                const SpecialAnimTag tag = config.skillTags[(size_t)idx];
                if (!IsSkillAvailableThisTurn(tag)) return;

                TimelineActionIntent intent{};
                if (BuildIntent_Skill(runtime.leaderEntity, tag, intent))
                {
                    if (timelineSys.TryCommitIntent(runtime.leaderEntity, intent))
                    {
                        MarkSkillUsedThisTurn(tag);
                        runtime.queuedSkillSlotFlags[(size_t)idx] = true;
                    }
                }
            };

        for (KEY k : config.keymap.skill.skillKeys) tryCommitSkill(k);
        return;
    }
    else
    {
        auto& timelineSys = registry.Get<BattleTimelineSystem>();

        if (input.KeyDown(config.keymap.primary.defend))
        {
            TimelineActionIntent intent{};
            if (isReady && BuildIntent_Defend(runtime.leaderEntity, intent))
                (void)timelineSys.TryCommitIntent(runtime.leaderEntity, intent);
            return;
        }

        if (input.KeyPressing(config.keymap.primary.flee))
        {
            if (!runtime.isEscapeHolding)
            {
                runtime.isEscapeHolding = true;
                runtime.escapeHoldStartSec = g_controllerTimeSec;
            }
            const double heldSec = g_controllerTimeSec - runtime.escapeHoldStartSec;
            if (heldSec >= config.tuning.escapeHoldNeedSec)
            {
                TimelineActionIntent intent{};
                if (BuildIntent_Escape(runtime.leaderEntity, intent))
                    (void)timelineSys.TryCommitIntent(runtime.leaderEntity, intent);
                runtime.isEscapeHolding = false;
            }
            return;
        }
        else
        {
            runtime.isEscapeHolding = false;
        }

        if (!isReady) return;

        if (input.KeyDown(config.keymap.primary.attack))
        {
            TimelineActionIntent intent{};
            if (BuildIntent_Basic(runtime.leaderEntity, intent))
                (void)timelineSys.TryCommitIntent(runtime.leaderEntity, intent);
            return;
        }

        if (input.KeyDown(config.keymap.primary.item))
        {
            TimelineActionIntent intent{};
            if (BuildIntent_Skill(runtime.leaderEntity, SpecialAnimTag::ItemRush, intent))
                (void)timelineSys.TryCommitIntent(runtime.leaderEntity, intent);
            return;
        }
    }
}

void BattleControllerSystem::OnGaugeBecameFull()
{
    runtime.turn.ResetForThisTurn();
    ResetTurnVisuals();
    ClearBuffer();
}

void BattleControllerSystem::OnActionExecutionFinished(const TimelineActionIntent& /*finishIntent*/)
{
    runtime.isExecuting = false;

    if (HasBuffered())
    {
        const TimelineActionIntent bufferedIntent = runtime.buffered.intent;
        ClearBuffer();
        (void)SubmitAccordingToPolicy(bufferedIntent);
    }
    else
        ResetTurnVisuals();
}

bool BattleControllerSystem::BuildIntent_Basic(EntityID leaderEntity, TimelineActionIntent& outIntent)
{
    if (leaderEntity == invalidEntity) return false;
    EntityID target{};
    if (!ResolveSingleTarget(leaderEntity, target)) return false;
    outIntent              = {};
    outIntent.battleCmd    = BattleCommand::AttackBasic;
    outIntent.targetEntity = target;
    outIntent.apCost       = 0;
    outIntent.specialTag   = SpecialAnimTag::BasicAttack;
    return true;
}

bool BattleControllerSystem::BuildIntent_Skill(EntityID leaderEntity, SpecialAnimTag tag, TimelineActionIntent& outIntent)
{
    if (leaderEntity == invalidEntity) return false;
    EntityID target{};
    if (!ResolveSingleTarget(leaderEntity, target)) return false;
    const int cost = registry.Get<BattleTimelineSystem>().ResolveSkillApCost(leaderEntity, tag);
    outIntent              = {};
    outIntent.battleCmd    = BattleCommand::Skill;
    outIntent.targetEntity = target;
    outIntent.apCost       = cost;
    outIntent.specialTag   = tag;
    return true;
}

bool BattleControllerSystem::BuildIntent_Defend(EntityID leaderEntity, TimelineActionIntent& outIntent)
{
    if (leaderEntity == invalidEntity) return false;
    outIntent              = {};
    outIntent.battleCmd    = BattleCommand::Defend;
    outIntent.targetEntity = leaderEntity;
    outIntent.apCost       = 0;
    return true;
}

bool BattleControllerSystem::BuildIntent_Escape(EntityID leaderEntity, TimelineActionIntent& outIntent)
{
    if (leaderEntity == invalidEntity) return false;
    outIntent              = {};
    outIntent.battleCmd    = BattleCommand::Escape;
    outIntent.targetEntity = leaderEntity;
    outIntent.apCost       = 0;
    return true;
}

bool BattleControllerSystem::ResolveSingleTarget(EntityID leaderEntity, EntityID& outTarget) const
{
    auto& sessionSys = registry.Get<BattleSessionSystem>();
    const BattleParty* allies = sessionSys.GetAllies();
    const BattleEnemies* enemies = sessionSys.GetEnemies();
    if (!allies || !enemies) return false;

    BattleTeam myTeam{};
    if (sessionSys.TryGetTeam(leaderEntity, myTeam))
    {
        if (myTeam == BattleTeam::Ally)
        {
            if (enemies->memberCount > 0 && enemies->members[0] != invalidEntity)
            {
                outTarget = enemies->members[0]; 
                return true;
            }
        }
        else
        {
            if (allies->memberCount > 0 && allies->members[0] != invalidEntity) 
            { 
                outTarget = allies->members[0]; 
                return true;
            }
        }
        return false;
    }
    if (enemies->memberCount > 0 && enemies->members[0] != invalidEntity) { outTarget = enemies->members[0]; return true; }
    return false;
}

void BattleControllerSystem::PushToBuffer(const TimelineActionIntent& intent)
{
    if (!runtime.buffered.hasValue)
    { 
        runtime.buffered.hasValue = true;
        runtime.buffered.intent = intent;
    }
}