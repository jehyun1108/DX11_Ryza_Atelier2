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

    runtime.leaderEntity = timelineSys.GetLeader();
    if (runtime.leaderEntity == invalidEntity)
    {
        runtime.menu.page       = CommandMenuPage::Hidden;
        runtime.isDefendingHold = false;
        runtime.isEscapeHolding = false;
        return;
    }

    HandleLeaderSwitching();
    HandleDefendHold(g_controllerTimeSec);
    if (runtime.isDefendingHold) return;

    HandleEscapeHold(g_controllerTimeSec);
    if (runtime.isEscapeHolding) return;

    const bool isReady = IsUnitReadyToAct(runtime.leaderEntity);
    HandleActionMenusAndCommit(isReady);
}

void BattleControllerSystem::OnGaugeBecameFull()
{
    runtime.turn.ResetForThisTurn();
    ResetTurnVisuals();
    ClearBuffer();
}

void BattleControllerSystem::OnActionExecutionFinished(const TimelineActionIntent& intent)
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
    auto& sessionSys             = registry.Get<BattleSessionSystem>();
    const BattleParty*   allies  = sessionSys.GetAllies();
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

void BattleControllerSystem::HandleLeaderSwitching()
{
    auto& input = registry.Get<InputService>();
    
    auto trySwitchTo = [&](int idx)
        {
            EntityID candidate = PickAllyByIdx(idx);
            if (candidate == invalidEntity) return;

            EntityID prev = runtime.leaderEntity;
            if (timelineSys.TrySetLeader(candidate))
            {
                CleanupPrevLeaderIfDefending(prev);
                runtime.isDefendingHold = false;
                ClearBuffer();
            }
        };

    if (input.KeyDown(KEY::NUM1)) trySwitchTo(0);
    if (input.KeyDown(KEY::NUM2)) trySwitchTo(1);
    if (input.KeyDown(KEY::NUM3)) trySwitchTo(2);
}

void BattleControllerSystem::HandleDefendHold(float t)
{
    auto& input = registry.Get<InputService>();
    auto& exec = registry.Get<BattleExecutionSystem>();

    const bool defendHolding = input.KeyPressing(config.keymap.primary.defend);
    if (defendHolding && !timelineSys.IsDefendAllowed(runtime.leaderEntity)) return;

    if (defendHolding && !runtime.isDefendingHold)
    {
        runtime.isDefendingHold = true;

        TimelineActionIntent intent{};
        if (BuildIntent_Defend(runtime.leaderEntity, intent))
        {
            exec.BeginAction(runtime.leaderEntity, intent);
            timelineSys.SetUnitGate(runtime.leaderEntity, TimelineUnitGate::Closed);
            timelineSys.SetUnitCanAction(runtime.leaderEntity, false);
            timelineSys.FreezeATB(runtime.leaderEntity, true);
        }
    }
    else if (!defendHolding && runtime.isDefendingHold)
    {
        runtime.isDefendingHold = false;

        TimelineActionIntent endIntent{};
        endIntent.battleCmd = BattleCommand::Defend;
        endIntent.specialTag = SpecialAnimTag::DefendEnd;

        exec.BeginAction(runtime.leaderEntity, endIntent);
        timelineSys.SetUnitGate(runtime.leaderEntity, TimelineUnitGate::Open);
        timelineSys.SetUnitCanAction(runtime.leaderEntity, true);
        timelineSys.FreezeATB(runtime.leaderEntity, false);
    }
}

void BattleControllerSystem::HandleEscapeHold(float t)
{
    auto& input = registry.Get<InputService>();

    if (input.KeyPressing(config.keymap.primary.flee))
    {
        if (!runtime.isEscapeHolding)
        {
            runtime.isEscapeHolding    = true;
            runtime.escapeHoldStartSec = t;
        }
        const double heldSec = t - runtime.escapeHoldStartSec;
        if (heldSec >= config.tuning.escapeHoldNeedSec)
        {
            auto& timeline = registry.Get<BattleTimelineSystem>();
            TimelineActionIntent intent{};
            if (BuildIntent_Escape(runtime.leaderEntity, intent))
                (void)timeline.TryCommitIntent(runtime.leaderEntity, intent);
            runtime.isEscapeHolding = false;
        }
    }
    else
        runtime.isEscapeHolding = false;
}

void BattleControllerSystem::HandleActionMenusAndCommit(bool isReady)
{
    auto& input = registry.Get<InputService>();
    const bool isSkillPageHeld = input.KeyPressing(KEY::SPACE);
    runtime.menu.page = isSkillPageHeld ? CommandMenuPage::Skill : CommandMenuPage::Primary;

    if (runtime.menu.page == CommandMenuPage::Skill)
        HandleSkillPage(isReady);
    else
        HandlePrimaryPage(isReady);
}

void BattleControllerSystem::HandleSkillPage(bool isReady)
{
    if (!isReady) return;

    auto& input = registry.Get<InputService>();

    auto tryCommitSkillKey = [&](KEY key)
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

    for (KEY k : config.keymap.skill.skillKeys)
        tryCommitSkillKey(k);
}

void BattleControllerSystem::HandlePrimaryPage(bool isReady)
{
    if (!isReady) return;

    auto& input = registry.Get<InputService>();

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

void BattleControllerSystem::CleanupPrevLeaderIfDefending(EntityID prevLeader)
{
    if (prevLeader == invalidEntity) return;
    if (!runtime.isDefendingHold)    return;

    auto& exec = registry.Get<BattleExecutionSystem>();

    TimelineActionIntent endIntent{};
    endIntent.battleCmd  = BattleCommand::Defend;
    endIntent.specialTag = SpecialAnimTag::DefendEnd;
    exec.BeginAction(prevLeader, endIntent);

    timelineSys.SetUnitGate(prevLeader, TimelineUnitGate::Open);
    timelineSys.SetUnitCanAction(prevLeader, true);
    timelineSys.FreezeATB(prevLeader, false);
}

EntityID BattleControllerSystem::PickAllyByIdx(int idx) const
{
    auto& session = registry.Get<BattleSessionSystem>();
    const BattleParty* allies = session.GetAllies();
    if (!allies) return invalidEntity;
    if (idx < 0 || idx >= allies->memberCount) return invalidEntity;
    return allies->members[idx];
}