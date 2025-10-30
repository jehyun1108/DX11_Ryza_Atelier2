#include "Enginepch.h"

bool BattleExecutionSystem::BeginAction(EntityID entity, const TimelineActionIntent& intent, ExecutionUnitRunTime& runtime)
{
	assert(intent.battleCmd == BattleCommand::AttackBasic || intent.battleCmd == BattleCommand::Skill);

	runtime.activeIntent       = intent;
	runtime.cursor.curChainIdx = 0;
	runtime.cursor.curStageIdx = 0;
	runtime.cursor.isActive    = true;

	const auto& animData       = registry.Get<AnimDataSystem>();
	const auto& actionRegistry = registry.Get<ActionAnimRegistry>();

	const ActionAnimSpec* actionSpec = actionRegistry.TryGet(runtime.character);
	assert(actionSpec && "ActionSpec not registered for this character");

	// ChainRule
	const AnimChainSpec* chosenChain{};
	if (intent.battleCmd == BattleCommand::AttackBasic)
	{
		assert(!actionSpec->basicAttackCombo.chainByOrder.empty());
		assert(actionSpec->basicAttackCombo.chainByOrder.size() == 1 && "No enforce single-chain basic combo");
		chosenChain = &actionSpec->basicAttackCombo.chainByOrder[0];
	}
	else if (intent.battleCmd == BattleCommand::Skill)
	{
		auto it = actionSpec->skillByKey.find(intent.skillKey);
		assert(it != actionSpec->skillByKey.end() && "Skill key not found");
		chosenChain = &it->second.chain;
	}
	return PlayStage(entity, runtime, *chosenChain);
}

void BattleExecutionSystem::Tick(EntityID entity, float dt, ExecutionUnitRunTime& runtime)
{
	if (!runtime.cursor.isActive) return;

	const auto& animData       = registry.Get<AnimDataSystem>();
	const auto& actionRegistry = registry.Get<ActionAnimRegistry>();
	const ActionAnimSpec* actionSpec = actionRegistry.TryGet(runtime.character);
	assert(actionSpec);

	const AnimChainSpec* chain = ResolveActiveChain(*actionSpec, runtime.activeIntent);
	assert(chain);

	if (IsCurStageFinished(entity, runtime))
	{
		++runtime.cursor.curStageIdx;
		if (runtime.cursor.curStageIdx >= static_cast<int>(chain->stages.size()))
		{
			runtime.cursor.isActive = false;
			registry.Get<BattleTimelineSystem>().NotifyActionFinished(entity, runtime.activeIntent);
			return;
		}
		PlayStage(entity, runtime, *chain);
	}
}

const AnimChainSpec* BattleExecutionSystem::ResolveActiveChain(const ActionAnimSpec& spec, const TimelineActionIntent& intent) const
{
	if (intent.battleCmd == BattleCommand::AttackBasic)
	{
		assert(spec.basicAttackCombo.chainByOrder.size() == 1);
		return &spec.basicAttackCombo.chainByOrder[0];
	}
	auto it = spec.skillByKey.find(intent.skillKey);
	return (it == spec.skillByKey.end()) ? nullptr : &it->second.chain;
}

bool BattleExecutionSystem::PlayStage(EntityID entity, ExecutionUnitRunTime& runtime, const AnimChainSpec& chain)
{
	assert(runtime.cursor.curStageIdx >= 0 && runtime.cursor.curStageIdx < static_cast<int>(chain.stages.size()));
	const AnimStageSpec& stage = chain.stages[runtime.cursor.curStageIdx];
	const auto& animData = registry.Get<AnimDataSystem>();

	// AnimKey -> ClipName
	const wstring& clipName = animData.GetClipName(runtime.character, runtime.context, stage.clipKey);
	assert(!clipName.empty() && "Clip name not found for AnimKey");

	auto& animator = registry.Get<AnimatorSystem>();

	const float fadeDur = max(0.f, stage.fadeDur);
	if (fadeDur > 0.f)
		animator.PlayFade(ResolveAnimHandle(entity), 0, clipName, fadeDur, 1.f, ANIMTYPE::ONCE);
	else
		animator.Play(ResolveAnimHandle(entity), 0, clipName, ANIMTYPE::ONCE);
	
	return true;
}

bool BattleExecutionSystem::IsCurStageFinished(EntityID entity, const ExecutionUnitRunTime& runtime) const
{
	if (!runtime.cursor.isActive) return true;

	const auto& actionRegistry = registry.Get<ActionAnimRegistry>();
	const ActionAnimSpec* actionSpec = actionRegistry.TryGet(runtime.character);
	assert(actionSpec && "IsCurStageFinished: ActionSpec not registered for this character");

	const AnimChainSpec* chain = ResolveActiveChain(*actionSpec, runtime.activeIntent);
	assert(chain && "IsCurStageFinished: Active chain not resolved");
	assert(runtime.cursor.curStageIdx >= 0 && runtime.cursor.curStageIdx < static_cast<int>(chain->stages.size()));

	const AnimStageSpec& stageSpec = chain->stages[runtime.cursor.curStageIdx];

	// AnimKey->ClipName
	const auto& animData    = registry.Get<AnimDataSystem>();
	const wstring& clipName = animData.GetClipName(runtime.character, runtime.context, stageSpec.clipKey);
	assert(!clipName.empty() && "IsCurStageFinished: Clip name not found for AnimKey");

	auto& animator    = registry.Get<AnimatorSystem>();
	Handle animHandle = ResolveAnimHandle(entity);

	if (!animator.IsPlaying(animHandle, 0)) return true;

	const bool  isCurClip    = animator.IsPlayingClip(animHandle, 0, clipName);
	const float remainingDur = animator.GetRemainingTime(animHandle, 0);

	if (isCurClip)
		return remainingDur <= 0.01f;
	else
		return remainingDur <= 0.01f;
}

Handle BattleExecutionSystem::ResolveAnimHandle(EntityID entity) const
{
	auto& animator = registry.Get<AnimatorSystem>();
	Handle handle{};
	animator.GetByOwner(entity, &handle);
	assert(handle.IsValid() && "Animator handle not found for entity");
	return handle;
}