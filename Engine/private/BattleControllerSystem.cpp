#include "Enginepch.h"

void BattleControllerSystem::Update(EntityID leaderEntity, float dt)
{
	auto& input = registry.Get<InputService>();

	runtime.leaderEntity = leaderEntity;
	if (leaderEntity == invalidEntity) return;

	const bool isDefendPressed = input.KeyPressing(config.primary.defendKey);
	if ( isDefendPressed && !runtime.isDefendingHold) HandleDefendHoldBegin();
	if (!isDefendPressed &&  runtime.isDefendingHold) HandleDefendHoldEnd();

	if (input.KeyDown(config.primary.basicAttackKey))   HandlePrimaryKeyDown(config.primary.basicAttackKey);
	if (input.KeyDown(config.primary.escapeKey))        HandlePrimaryKeyDown(config.primary.escapeKey);
	if (input.KeyDown(config.primary.openSkillMenuKey)) HandlePrimaryKeyDown(config.primary.openSkillMenuKey);

	for (const auto& slot : config.quickSkills)
	{
		if (input.KeyDown(slot.key))
			HandleSkillSlotKeyDown(slot.key);
	}
}

void BattleControllerSystem::OnGaugeBecameFull()
{
	runtime.turn.ResetForThisTurn();
	ResetTurnVisuals();
	ClearBuffer();
}

void BattleControllerSystem::OnActionExecutionFinished(const TimelineActionIntent& finishIntent)
{
	runtime.isExecuting = false;

	if (HasBuffered())
	{
		const TimelineActionIntent bufferedIntent = runtime.buffered.intent;
		ClearBuffer();
		SubmitAccordingToPolicy(bufferedIntent);
	}
	else
		ResetTurnVisuals();
}

void BattleControllerSystem::HandlePrimaryKeyDown(KEY pressedKey)
{
	if (runtime.isExecuting && HasBuffered()) return;
	if (!IsGaugeFull(runtime.leaderEntity)) return;

	if (pressedKey == config.primary.basicAttackKey)
	{
		TimelineActionIntent intent{};
		if (!BuildIntent_Basic(runtime.leaderEntity, intent)) return;

		if (runtime.isExecuting)  PushToBuffer(intent);
		else                      SubmitAccordingToPolicy(intent);
		return;
	}
	if (pressedKey == config.primary.escapeKey)
	{
		TryEscape();
		return;
	}
	if (pressedKey == config.primary.openSkillMenuKey)
	{
		EnterSelectingSkill();
		return;
	}
}

void BattleControllerSystem::HandleSkillSlotKeyDown(KEY pressedKey)
{
	if (runtime.isExecuting && HasBuffered()) return;

	const QuickSkillBindings* chosenSlot = nullptr;
	for (const auto& slot : config.quickSkills)
	{
		if (slot.key == pressedKey) { chosenSlot = &slot; break; }
	}
	if (!chosenSlot)                                     return;
	if (!IsGaugeFull(runtime.leaderEntity))              return;
	if (!IsSkillAvailableThisTurn(chosenSlot->tag)) return;

	TimelineActionIntent intent{};
	if (!BuildIntent_Skill(runtime.leaderEntity, chosenSlot->tag, intent)) return;

	if (runtime.isExecuting)
	{
		PushToBuffer(intent);
		MarkSkillSlotQueuedByKey(chosenSlot->tag);
		MarkSkillUsedThisTurn(chosenSlot->tag);
	}
	else
	{
		if (SubmitAccordingToPolicy(intent))
		{
			MarkSkillSlotQueuedByKey(chosenSlot->tag);
			MarkSkillUsedThisTurn(chosenSlot->tag);
		}
	}
	
	if (runtime.mode == ControllerMode::Skill)
		ExitSelectingSkill();
}

void BattleControllerSystem::TryEscape()
{
	TimelineActionIntent intent{};
	if (!BuildIntent_Escape(runtime.leaderEntity, intent)) return;

	if (runtime.isExecuting) PushToBuffer(intent);
	else                     SubmitAccordingToPolicy(intent);
}

void BattleControllerSystem::StartSwapLeader(EntityID newLeaderEntity)
{
	TimelineActionIntent intent{};
	intent.battleCmd    = BattleCommand::SwapLeader;
	intent.targetEntity = newLeaderEntity;
	intent.apCost       = 0;
	//intent.skillKey     = L"";

	if (runtime.isExecuting) PushToBuffer(intent);
	else                     SubmitAccordingToPolicy(intent);

	runtime.mode = ControllerMode::SwapLeader;
}

void BattleControllerSystem::SubmitIntent(const TimelineActionIntent& intent)
{
	if (runtime.isExecuting) PushToBuffer(intent);
	else                     SubmitAccordingToPolicy(intent);
}

bool BattleControllerSystem::IsGaugeFull(EntityID entity) const
{
	auto& timelineSys = registry.Get<BattleTimelineSystem>();
	return timelineSys.IsGaugeFull(entity);
}

bool BattleControllerSystem::BuildIntent_Basic(EntityID leaderEntity, TimelineActionIntent& outIntent)
{
	if (leaderEntity == invalidEntity) return false;

	EntityID resolvedTarget{};
	if (!ResolveSingleTarget(leaderEntity, resolvedTarget)) return false;

	outIntent              = {};
	outIntent.battleCmd    = BattleCommand::AttackBasic;
	outIntent.targetEntity = resolvedTarget;
	outIntent.apCost       = 0;
	outIntent.specialTag   = SpecialAnimTag::BasicAttack;
	return true;
}

bool BattleControllerSystem::BuildIntent_Skill(EntityID leaderEntity, SpecialAnimTag specialTag, TimelineActionIntent& outIntent)
{
	if (leaderEntity == invalidEntity) return false;

	EntityID resolvedTarget{};
	if (!ResolveSingleTarget(leaderEntity, resolvedTarget)) return false;

	auto& timelineSys = registry.Get<BattleTimelineSystem>();
	const int resolvedApCost = timelineSys.ResolveSkillApCost(leaderEntity, specialTag);

	outIntent              = {};
	outIntent.battleCmd    = BattleCommand::Skill;   
	outIntent.targetEntity = resolvedTarget;
	outIntent.apCost       = resolvedApCost;
	outIntent.specialTag   = specialTag;            
	return true;
}

bool BattleControllerSystem::BuildIntent_Defend(EntityID leaderEntity, TimelineActionIntent& outIntent)
{
	if (leaderEntity == invalidEntity) return false;

	outIntent              = {};
	outIntent.battleCmd    = BattleCommand::Defend;
	outIntent.targetEntity = leaderEntity;
	outIntent.apCost       = 0;
	//outIntent.skillKey     = L"";
	return true;
}

bool BattleControllerSystem::BuildIntent_Escape(EntityID leaderEntity, TimelineActionIntent& outIntent)
{
	if (leaderEntity == invalidEntity) return false;

	outIntent              = {};
	outIntent.battleCmd    = BattleCommand::Escape;
	outIntent.targetEntity = leaderEntity;
	outIntent.apCost       = 0;
	//outIntent.skillKey     = L"";
	return true;
}

bool BattleControllerSystem::ResolveSingleTarget(EntityID leaderEntity, EntityID& outTarget) const
{
	auto& sessionSys = registry.Get<BattleSessionSystem>();
	const BattleParty*   allies  = sessionSys.GetAllies();
	const BattleEnemies* enemies = sessionSys.GetEnemies();
	if (!allies || !enemies) return false;

	BattleTeam team{};
	if (sessionSys.TryGetTeam(leaderEntity, team))
	{
		if (team == BattleTeam::Ally)
		{
			if (enemies->memberCount > 0 && enemies->members[0] != invalidEntity)
			{
				outTarget = enemies->members[0];
				return true;
			}
		}
		else if (team == BattleTeam::Enemy)
		{
			if (allies->memberCount > 0 && allies->members[0] != invalidEntity)
			{
				outTarget = allies->members[0];
				return true;
			}
		}
		return false;
	}

	if (enemies->memberCount > 0 && enemies->members[0] != invalidEntity)
	{
		outTarget = enemies->members[0];
		return true;
	}
	return false;
}

bool BattleControllerSystem::SubmitAccordingToPolicy(const TimelineActionIntent& intent)
{
	auto& timelineSys = registry.Get<BattleTimelineSystem>();

	const bool enqueued = timelineSys.EnqueuePlayerIntent(runtime.leaderEntity, intent);
	if (!enqueued) return false;

	if (config.submitPolicy == SubmitPolicy::AutoCommitIfReady)
		(void)timelineSys.TryCommitIntent(runtime.leaderEntity, intent);

	return true;
}

void BattleControllerSystem::PushToBuffer(const TimelineActionIntent& intent)
{
	if (!runtime.buffered.hasValue)
	{
		runtime.buffered.hasValue = true;
		runtime.buffered.intent = intent;
	}
}

int BattleControllerSystem::FindQuickSlotIdxByKey(SpecialAnimTag tag) const
{
	for (int idx = 0; idx < static_cast<int>(config.quickSkills.size()); ++idx)
	{
		if (config.quickSkills[static_cast<size_t>(idx)].tag == tag)
			return idx;
	}
	return -1;
}

void BattleControllerSystem::MarkSkillSlotQueuedByKey(SpecialAnimTag tag)
{
	const int idx = FindQuickSlotIdxByKey(tag);
	if (idx >= 0 && idx < static_cast<int>(runtime.queuedSkillSlotFlags.size()))
		runtime.queuedSkillSlotFlags[static_cast<size_t>(idx)] = true;
}