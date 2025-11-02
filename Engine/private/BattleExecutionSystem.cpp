#include "Enginepch.h"

bool BattleExecutionSystem::BeginAction(EntityID entity, const TimelineActionIntent& intent, ExecutionUnitRunTime& runtime)
{
	runtime.activeIntent = intent;
	runtime.cursor.curChainIdx = 0;
	runtime.cursor.curStageIdx = 0;
	runtime.cursor.isActive = true;

	const auto& actionRegistry = registry.Get<ActionAnimRegistry>();
	const ActionAnimSpec* actionSpec = actionRegistry.TryGet(runtime.character);
	if (!actionSpec)
	{
		runtime.cursor.isActive = false;
		registry.Get<BattleTimelineSystem>().NotifyActionFinished(entity, runtime.activeIntent);
		return false;
	}

	const AnimChainSpec* activeChain = ResolveActiveChain(*actionSpec, intent);
	if (!activeChain || activeChain->stages.empty())
	{
		runtime.cursor.isActive = false;
		registry.Get<BattleTimelineSystem>().NotifyActionFinished(entity, runtime.activeIntent);
		return false;
	}

	return PlayStage(entity, runtime, *activeChain);
}

void BattleExecutionSystem::Tick(EntityID entity, float dt, ExecutionUnitRunTime& runtime)
{
	if (!runtime.cursor.isActive) return;

	auto& actionRegistry = registry.Get<ActionAnimRegistry>();
	auto& timelineSys    = registry.Get<BattleTimelineSystem>();

	const ActionAnimSpec* actionSpec = actionRegistry.TryGet(runtime.character);
	if (!actionSpec)
	{
		runtime.cursor.isActive = false;
		timelineSys.NotifyActionFinished(entity, runtime.activeIntent);
		return;
	}

	const AnimChainSpec* activeChain = ResolveActiveChain(*actionSpec, runtime.activeIntent);
	if (!activeChain || activeChain->stages.empty())
	{
		runtime.cursor.isActive = false;
		timelineSys.NotifyActionFinished(entity, runtime.activeIntent);
		return;
	}

	if (IsCurStageFinished(entity, runtime, *activeChain))
	{
		++runtime.cursor.curStageIdx;
		if (runtime.cursor.curStageIdx >= static_cast<int>(activeChain->stages.size()))
		{
			runtime.cursor.isActive = false;
			timelineSys.NotifyActionFinished(entity, runtime.activeIntent);
			return;
		}
		PlayStage(entity, runtime, *activeChain);
	}
}

const AnimChainSpec* BattleExecutionSystem::ResolveActiveChain(const ActionAnimSpec& spec, const TimelineActionIntent& intent) const
{
	if (!intent.specialTag.has_value()) return nullptr;
	auto it = spec.specials.find(intent.specialTag.value());
	return (it == spec.specials.end()) ? nullptr : &it->second;
}

bool BattleExecutionSystem::PlayStage(EntityID entity, ExecutionUnitRunTime& runtime, const AnimChainSpec& chain)
{
	auto& timelineSys = registry.Get<BattleTimelineSystem>();

	if (runtime.cursor.curStageIdx < 0 || runtime.cursor.curStageIdx >= static_cast<int>(chain.stages.size()))
	{
		runtime.cursor.isActive = false;
		timelineSys.NotifyActionFinished(entity, runtime.activeIntent);
		return false;
	}

	const AnimStageSpec& stageSpec = chain.stages[runtime.cursor.curStageIdx];

	const auto& animData = registry.Get<AnimDataSystem>();
	const wstring& clipName = animData.GetClipName(runtime.character, runtime.context, stageSpec.clipKey);
	if (clipName.empty())
	{
		runtime.cursor.isActive = false;
		timelineSys.NotifyActionFinished(entity, runtime.activeIntent);
		return false;
	}

	ClipTuning tuning = animData.GetClipTuning(runtime.character, runtime.context, stageSpec.clipKey);

	if (stageSpec.startNormalizedOverride)
		tuning.startNormalized = *stageSpec.startNormalizedOverride;
	if (stageSpec.endNormalizedOverride)
		tuning.endNormalized   = *stageSpec.endNormalizedOverride;


	auto& animator = registry.Get<AnimatorSystem>();
	Handle animHandle = ResolveAnimHandle(entity);
	if (!animHandle.IsValid())
	{
		runtime.cursor.isActive = false;
		timelineSys.NotifyActionFinished(entity, runtime.activeIntent);
		return false;
	}

	const float fadeSeconds = max(0.f, stageSpec.fadeDur);
	animator.SetPlaybackSpeed(animHandle, 0, tuning.playbackSpeed);
	if (fadeSeconds > 0.f)
		animator.CrossFade(animHandle, 0, 1, clipName, fadeSeconds, ANIMTYPE::ONCE, tuning.startNormalized, tuning.endNormalized);
	else
		animator.PlaySection(animHandle, 0, clipName, ANIMTYPE::ONCE, tuning.startNormalized, tuning.endNormalized);

	return true;
}

bool BattleExecutionSystem::IsCurStageFinished(EntityID entity, const ExecutionUnitRunTime& runtime, const AnimChainSpec& chain) const
{
	if (!runtime.cursor.isActive) return true;
	if (runtime.cursor.curStageIdx < 0 || runtime.cursor.curStageIdx >= static_cast<int>(chain.stages.size())) return true;

	const AnimStageSpec& stageSpec = chain.stages[runtime.cursor.curStageIdx];

	const auto& animData = registry.Get<AnimDataSystem>();
	const wstring& clipName = animData.GetClipName(runtime.character, runtime.context, stageSpec.clipKey);
	if (clipName.empty()) return true;

	auto& animator = registry.Get<AnimatorSystem>();
	Handle animHandle = ResolveAnimHandle(entity);
	if (!animHandle.IsValid()) return true;
	if (!animator.IsPlaying(animHandle, 0)) return true;

	if (animator.IsCrossFading(animHandle)) return false;

	const float remainingSeconds = animator.GetRemainingTime(animHandle, 0);
	const float transitionWindow = max(stageSpec.fadeDur, stageSpec.minOverlapDur);
	return (remainingSeconds <= transitionWindow);
}

Handle BattleExecutionSystem::ResolveAnimHandle(EntityID entity) const
{
	auto& animator = registry.Get<AnimatorSystem>();
	Handle handle{};
	animator.GetByOwner(entity, &handle);
	return handle;
}