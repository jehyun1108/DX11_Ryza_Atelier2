#include "Enginepch.h"
#include "SoundSystem.h"

static double g_controllerTimeSec = 0.0;
static inline int FindIndexIn(const array<KEY, 4>& keys, KEY k)
{
    for (int i = 0; i < 4; ++i) 
        if (keys[(size_t)i] == k) 
            return i;
    return -1;
}
// ------------------------------------------------------------------------------------------------------------------------------
void BattleControllerSystem::OnBoot()
{
    timelineSys = &registry.Get<BattleTimelineSystem>();
    sessionSys  = &registry.Get<BattleSessionSystem>();
    input       = &registry.Get<InputService>();
    execSys     = &registry.Get<BattleExecutionSystem>();
    animReg     = &registry.Get<ActionAnimRegistry>();
    dataSys     = &registry.Get<CharacterDataSystem>();
    soundSys    = &registry.Get<SoundSystem>();
}
void BattleControllerSystem::Update(float dt)
{
    g_controllerTimeSec += static_cast<double>(dt);
    runtime.leaderEntity = timelineSys->GetLeader();

    HandleLeaderSwitching();
    HandleDefendHold();
    if (runtime.isDefendingHold) return;

    HandleEscapeHold((float)g_controllerTimeSec);
    if (runtime.isEscapeHolding) return;

    const bool isReady = IsUnitReadyToAct(runtime.leaderEntity);
    const bool isComboWindow = execSys->IsComboInputOpen(runtime.leaderEntity);

    HandleActionMenusAndCommit(isReady, isComboWindow);
}

void BattleControllerSystem::OnGaugeBecameFull()
{
    ResetTurnVisuals();
    ClearBuffer();

    runtime.usedStepCountPerSlot = { 0, 0, 0, 0 };
    runtime.slotLockedThisTurn = { false, false, false, false };
    runtime.primarySlotIndexThisTurn = -1;
}

void BattleControllerSystem::OnActionExecutionStarted(EntityID entity, const TimelineActionIntent& startIntent)
{
    if (entity != runtime.leaderEntity) return;
    if (startIntent.battleCmd == BattleCommand::Defend) return;
    runtime.isExecuting = true;
}

void BattleControllerSystem::OnActionExecutionFinished(EntityID entity, const TimelineActionIntent& intent)
{
    if (entity != runtime.leaderEntity) return;
    if (intent.battleCmd == BattleCommand::Defend)  return;
    runtime.isExecuting = false;
    ClearBuffer();     
    ResetTurnVisuals();
}

TimelineActionIntent BattleControllerSystem::BuildIntent_Basic(EntityID leaderEntity)
{
    const EntityID target = ResolveSingleTarget(leaderEntity);
    TimelineActionIntent intent{};
    intent.battleCmd = BattleCommand::AttackBasic;
    intent.targetEntity = target;
    intent.apCost = 0;
    intent.specialTag = SpecialAnimTag::BasicAttack;
    return intent;
}

TimelineActionIntent BattleControllerSystem::BuildIntent_Skill(EntityID leaderEntity, SkillSlotTag slot)
{
    int slotIndex = 0;
    switch (slot)
    {
    case SkillSlotTag::A: slotIndex = 0; break;
    case SkillSlotTag::B: slotIndex = 1; break;
    case SkillSlotTag::C: slotIndex = 2; break;
    case SkillSlotTag::D: slotIndex = 3; break;
    }

    int step = runtime.usedStepCountPerSlot[(size_t)slotIndex];
    CharacterID character = dataSys->GetCharacterID(leaderEntity);
    SpecialAnimTag stepTag = animReg->GetStepTag(character, slot, step);

    const EntityID target = ResolveSingleTarget(leaderEntity);
    const int      cost = timelineSys->ResolveSkillApCost(leaderEntity, stepTag);

    TimelineActionIntent intent{};
    intent.battleCmd = BattleCommand::Skill;
    intent.targetEntity = target;
    intent.apCost = cost;
    intent.specialTag = stepTag;
    return intent;
}

TimelineActionIntent BattleControllerSystem::BuildIntent_Defend(EntityID leaderEntity)
{
    TimelineActionIntent intent{};
    intent.battleCmd = BattleCommand::Defend;
    intent.targetEntity = leaderEntity;
    intent.apCost = 0;
    intent.specialTag = SpecialAnimTag::DefendStart;
    return intent;
}

TimelineActionIntent BattleControllerSystem::BuildIntent_Escape(EntityID leaderEntity)
{
    TimelineActionIntent intent{};
    intent.battleCmd = BattleCommand::Escape;
    intent.targetEntity = leaderEntity;
    intent.apCost = 0;
    return intent;
}

TimelineActionIntent BattleControllerSystem::BuildIntent_ItemRush(EntityID leaderEntity)
{
    TimelineActionIntent intent{};
    intent.battleCmd = BattleCommand::Skill;
    intent.targetEntity = ResolveSingleTarget(leaderEntity);
    intent.apCost = timelineSys->ResolveSkillApCost(leaderEntity, SpecialAnimTag::ItemRush);
    intent.specialTag = SpecialAnimTag::ItemRush;
    return intent;
}

EntityID BattleControllerSystem::ResolveSingleTarget(EntityID leaderEntity) const
{
    const BattleTeam myTeam = sessionSys->GetTeam(leaderEntity);
    const BattleParty& allies = sessionSys->GetAllies();
    const BattleEnemies& enemies = sessionSys->GetEnemies();

    if (myTeam == BattleTeam::Ally)
        return enemies.members[0];
    else
        return allies.members[0];
}

bool BattleControllerSystem::PushToBuffer(const TimelineActionIntent& intent)
{
    if (runtime.buffered.hasValue)
        return false;
    if (!execSys->CanQueueCombo(runtime.leaderEntity))
        return false;

    runtime.buffered.hasValue = true;
    runtime.buffered.intent = intent;

    if (intent.specialTag.has_value())
        execSys->NotifyComboQueued(runtime.leaderEntity, *intent.specialTag);

    return true;
}

void BattleControllerSystem::HandleLeaderSwitching()
{
    auto switchToIndex = [&](int idx)
        {
            const EntityID candidate = PickAllyByIdx(idx);
            if (candidate == 0u) return;

            const EntityID previous = runtime.leaderEntity;
            timelineSys->SetLeader(candidate);
            CleanupPrevLeaderIfDefending(previous);
            runtime.isDefendingHold = false;
            ClearBuffer();
        };

    if (input->KeyDown(KEY::R)) switchToIndex(0);
    if (input->KeyDown(KEY::K)) switchToIndex(1);
    if (input->KeyDown(KEY::P)) switchToIndex(2);
}

void BattleControllerSystem::HandleDefendHold()
{
    const bool holding = input->KeyPressing(config.keymap.primary.defend);
    if (holding && !timelineSys->IsDefendAllowed(runtime.leaderEntity)) return;

    if (holding && !runtime.isDefendingHold)
    {
        runtime.isDefendingHold = true;

        const TimelineActionIntent startIntent = BuildIntent_Defend(runtime.leaderEntity);
        execSys->BeginAction(runtime.leaderEntity, startIntent, false);
        timelineSys->SetUnitGate(runtime.leaderEntity, TimelineUnitGate::Closed);
        timelineSys->SetUnitCanAction(runtime.leaderEntity, false);
        timelineSys->FreezeATB(runtime.leaderEntity, true);
    }
    else if (!holding && runtime.isDefendingHold)
    {
        runtime.isDefendingHold = false;

        TimelineActionIntent endIntent{};
        endIntent.battleCmd = BattleCommand::Defend;
        endIntent.specialTag = SpecialAnimTag::DefendEnd;

        execSys->BeginAction(runtime.leaderEntity, endIntent, false);
        timelineSys->SetUnitGate(runtime.leaderEntity, TimelineUnitGate::Open);
        timelineSys->SetUnitCanAction(runtime.leaderEntity, true);
        timelineSys->FreezeATB(runtime.leaderEntity, false);
    }
}

void BattleControllerSystem::HandleEscapeHold(float t)
{
    if (input->KeyPressing(config.keymap.skill.openHold))
    {
        runtime.isEscapeHolding = false;
        return;
    }

    if (input->KeyPressing(config.keymap.primary.flee))
    {
        if (!runtime.isEscapeHolding)
        {
            runtime.isEscapeHolding = true;
            runtime.escapeHoldStartSec = t;
        }
        const double held = t - runtime.escapeHoldStartSec;
        if (held >= config.tuning.escapeHoldNeedSec)
        {
            const TimelineActionIntent intent = BuildIntent_Escape(runtime.leaderEntity);
            timelineSys->CommitIntent(runtime.leaderEntity, intent);
            runtime.isEscapeHolding = false;
        }
    }
    else
        runtime.isEscapeHolding = false;
}

void BattleControllerSystem::HandleActionMenusAndCommit(bool isReady, bool isComboWindow)
{
    const bool isSkillPageHeld = input->KeyPressing(config.keymap.skill.openHold);
    runtime.menu.page = isSkillPageHeld ? CommandMenuPage::Skill : CommandMenuPage::Primary;

    if (runtime.menu.page == CommandMenuPage::Skill)
        HandleSkillPage(isReady, isComboWindow);
    else
        HandlePrimaryPage(isReady);
}

void BattleControllerSystem::HandleSkillPage(bool isReady, bool isComboWindow)
{
    const bool isExecuting = runtime.isExecuting;
    const bool canStartNew = isReady && !isExecuting;             
    const bool canQueueCombo = isExecuting && isComboWindow;         

    if (!canStartNew && !canQueueCombo)
        return;

    auto tryCommitSkillKey = [&](KEY key)
        {
            if (!input->KeyDown(key)) return;

            const int idx = FindIndexIn(config.keymap.skill.skillKeys, key);
            if (idx < 0)
                return;

            if (runtime.slotLockedThisTurn[(size_t)idx]) 
                return;

            if (runtime.primarySlotIndexThisTurn < 0)
                runtime.primarySlotIndexThisTurn = idx;
            else if (runtime.primarySlotIndexThisTurn != idx)
            {
                runtime.slotLockedThisTurn[(size_t)runtime.primarySlotIndexThisTurn] = true;
                runtime.primarySlotIndexThisTurn = idx;
            }

            int step = runtime.usedStepCountPerSlot[(size_t)idx];
            if (step >= 3)
            {
                runtime.slotLockedThisTurn[(size_t)idx] = true;
                return;
            }

            SkillSlotTag slot =
                (idx == 0) ? SkillSlotTag::A :
                (idx == 1) ? SkillSlotTag::B :
                (idx == 2) ? SkillSlotTag::C :
                SkillSlotTag::D;

            TimelineActionIntent intent = BuildIntent_Skill(runtime.leaderEntity, slot);

            bool accepted = false;
            if (canStartNew)
            {
                timelineSys->CommitIntent(runtime.leaderEntity, intent);
                accepted = true;
            }
            else if (canQueueCombo)
            {
                accepted = PushToBuffer(intent); 
            }

            if (!accepted)
                return;

            if (!accepted)
                return;

            if (runtime.primarySlotIndexThisTurn < 0)
                runtime.primarySlotIndexThisTurn = idx;
            else if (runtime.primarySlotIndexThisTurn != idx)
            {
                runtime.slotLockedThisTurn[(size_t)runtime.primarySlotIndexThisTurn] = true;
                runtime.primarySlotIndexThisTurn = idx;
            }

            runtime.usedStepCountPerSlot[(size_t)idx] = step + 1;

            if (runtime.usedStepCountPerSlot[(size_t)idx] >= 3)
                runtime.slotLockedThisTurn[(size_t)idx] = true;

            runtime.queuedSkillSlotFlags[(size_t)idx] = true;
        };

    for (KEY k : config.keymap.skill.skillKeys)
        tryCommitSkillKey(k);
}

void BattleControllerSystem::HandlePrimaryPage(bool isReady)
{
    if (!isReady) return;

    if (input->KeyDown(config.keymap.primary.attack))
    {
        const TimelineActionIntent intent = BuildIntent_Basic(runtime.leaderEntity);
        timelineSys->CommitIntent(runtime.leaderEntity, intent);
        return;
    }

    if (input->KeyDown(config.keymap.primary.item))
    {
        const TimelineActionIntent intent = BuildIntent_ItemRush(runtime.leaderEntity);
        timelineSys->CommitIntent(runtime.leaderEntity, intent);
        return;
    }
}

void BattleControllerSystem::CleanupPrevLeaderIfDefending(EntityID prevLeader)
{
    if (prevLeader == 0u) return;
    if (!runtime.isDefendingHold)    return;

    TimelineActionIntent endIntent{};
    endIntent.battleCmd  = BattleCommand::Defend;
    endIntent.specialTag = SpecialAnimTag::DefendEnd;
    execSys->BeginAction(prevLeader, endIntent, false);

    timelineSys->SetUnitGate(prevLeader, TimelineUnitGate::Open);
    timelineSys->SetUnitCanAction(prevLeader, true);
    timelineSys->FreezeATB(prevLeader, false);
}

EntityID BattleControllerSystem::PickAllyByIdx(int idx) const
{
    const BattleParty& allies = sessionSys->GetAllies();
    if (idx < 0 || idx >= allies.memberCount) return 0u;
    return allies.members[idx];
}

void BattleControllerSystem::OnComboStepStarted(EntityID entity, SkillSlotTag slot, int stepIdx)
{
    if (entity != runtime.leaderEntity)
        return;

    runtime.buffered.hasValue = false;

    const size_t i = static_cast<size_t>(slot);
    const int used = stepIdx + 1;

    if (runtime.usedStepCountPerSlot[i] < used)
        runtime.usedStepCountPerSlot[i] = used;
}

void BattleControllerSystem::RenderGui(EntityID id)
{
#ifdef USE_IMGUI

    ImGui::Separator();
    ImGui::Text("BattleControllerSystem");

    EntityID leader = runtime.leaderEntity;
    bool isFocusLeader = (id != 0u && id == leader);

    ImGui::Text("Leader   : %u%s", leader, isFocusLeader ? "  <FOCUS>" : "");
    ImGui::Text("FocusEnt : %u", id);

    const char* pageName = "";
    switch (runtime.menu.page)
    {
    case CommandMenuPage::Hidden:  pageName = "Hidden";  break;
    case CommandMenuPage::Primary: pageName = "Primary"; break;
    case CommandMenuPage::Skill:   pageName = "Skill";   break;
    }
    ImGui::Text("MenuPage : %s", pageName);

    ImGui::Text("Flags:");
    ImGui::BulletText("isExecuting     = %s", runtime.isExecuting ? "true" : "false");
    ImGui::BulletText("isDefendingHold = %s", runtime.isDefendingHold ? "true" : "false");
    ImGui::BulletText("isEscapeHolding = %s", runtime.isEscapeHolding ? "true" : "false");

    bool isReady = false;
    bool isComboWindow = false;
    if (leader != 0u)
    {
        isReady = IsUnitReadyToAct(leader);
        isComboWindow = runtime.isExecuting && execSys->IsComboInputOpen(leader);
    }

    ImGui::Text("Turn State:");
    ImGui::BulletText("isReady       = %s", isReady ? "true" : "false");
    ImGui::BulletText("isComboWindow = %s", isComboWindow ? "true" : "false");

    ImGui::Text("Buffer:");
    ImGui::BulletText("hasBuffered = %s", runtime.buffered.hasValue ? "true" : "false");
    if (runtime.buffered.hasValue)
    {
        const TimelineActionIntent& b = runtime.buffered.intent;
        ImGui::BulletText("cmd=%d target=%u apCost=%d",
            static_cast<int>(b.battleCmd),
            b.targetEntity, 
            b.apCost);
        if (b.specialTag.has_value())
            ImGui::BulletText("specialTag=%d", static_cast<int>(*b.specialTag));
    }

    ImGui::Separator();
    ImGui::Text("Skill Slots (A/B/C/D):");

    for (int i = 0; i < 4; ++i)
    {
        const bool locked = runtime.slotLockedThisTurn[(size_t)i];
        const int  step = runtime.usedStepCountPerSlot[(size_t)i];
        const bool queued = runtime.queuedSkillSlotFlags[(size_t)i];
        const bool isPrimary = (runtime.primarySlotIndexThisTurn == i);

        const char* slotName =
            (i == 0) ? "A" :
            (i == 1) ? "B" :
            (i == 2) ? "C" : "D";

        ImGui::BulletText("Slot %s : step=%d  locked=%s  queued=%s%s",
            slotName,
            step,
            locked ? "true" : "false",
            queued ? "true" : "false",
            isPrimary ? "  <PRIMARY>" : "");
    }

#endif
}