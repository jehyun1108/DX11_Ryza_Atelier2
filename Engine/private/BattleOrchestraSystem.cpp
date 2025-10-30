#include "Enginepch.h"

void BattleOrchestraSystem::Enter()
{
	auto& input = registry.Get<InputService>();
	input.SetContext(InputContext::Battle);
	input.SetFocus(FocusState::UI);
	input.SetManualTime(0.f);

	eventBus.ReserveQueue(256);
	WireSubscriptions();
}

void BattleOrchestraSystem::Update(float dt)
{
	auto& sessionSys  = registry.Get<BattleSessionSystem>();
	auto& intro       = registry.Get<BattleIntroSystem>();
	auto& controller  = registry.Get<BattleControllerSystem>();
	auto& timelineSys = registry.Get<BattleTimelineSystem>();

	sessionSys.Update(dt);
	intro.Update(dt);

	// Session Event -> BusEvent 
	PumpSessionEventsToBus();

	if (sessionSys.GetPhase() == BattlePhase::Active)
	{
		controller.Update(sessionSys.GetLeader(), dt);
		timelineSys.Tick(dt);
		TickExecutions(dt);
		PumpTimelineEventsToBus();
	}

	eventBus.DispatchAll();
	eventBus.ClearQueue();
}

void BattleOrchestraSystem::Exit()
{
	UnwireSubscriptions();
	execRuntimeByEntity.clear();

	auto& input = registry.Get<InputService>();
	input.SetFocus(FocusState::None);
}

bool BattleOrchestraSystem::BeginBattle(const BattleStartParams& Inparams)
{
	auto  params      = Inparams;
	auto& sessionSys  = registry.Get<BattleSessionSystem>();
	auto& introSys    = registry.Get<BattleIntroSystem>();
	auto& tfSys       = registry.Get<TransformSystem>();
	auto& animSys     = registry.Get<AnimatorSystem>();
	auto& moveSys     = registry.Get<MoveStateSystem>();
	auto& dataSys     = registry.Get<CharacterDataSystem>();
	auto& timelineSys = registry.Get<BattleTimelineSystem>();

	sessionSys.BeginSession(params.allies, params.enemies, params.centerWorld, params.sessionConfig);
	sessionSys.SetSpacing(params.ringRadius);
	sessionSys.SetAllyStartAngleDeg(params.startAngleDeg);

	auto createIntroFor = [&](EntityID entity) 
		{
			if (entity == invalidEntity)  return;

			MoveState* move = moveSys.GetByOwner(entity);
			if (!move) return;

			Handle tfHandle = move->tfHandle;
			Handle animHandle{};
			animSys.GetByOwner(entity, &animHandle);
			if (!animHandle.IsValid()) return;

			AnimProfile profile = dataSys.ResolveProfile(entity, AnimContext::Battle);
			introSys.Create(entity, animHandle, tfHandle, profile);
		};

	for (int i = 0; i < params.allies.memberCount; ++i)  createIntroFor(params.allies.members[i]);
	for (int i = 0; i < params.enemies.memberCount; ++i) createIntroFor(params.enemies.members[i]);

	if (const BattleSessionState* state = sessionSys.TryGetState())
	{
		BattleTimelineConfig timelineConfig{};
		timelineSys.InitSession(*state, timelineConfig);
	}
	
	return true;
}

void BattleOrchestraSystem::WireSubscriptions()
{
	listenerIds.clear();

	// TimelineCommitted -> Execution BeginAction
	BattleEventListenerId idCommited = eventBus.Subscribe(BattleBusEventType::TimelineActionCommitted, [this](const BattleEvent& event) 
		{
			const EntityID subject = event.subjectEntity;
			if (subject == invalidEntity) return;

			TimelineActionIntent intent{};
			if (!TryFillIntentFromTimeline(subject, intent)) return;

			auto& dataSys = registry.Get<CharacterDataSystem>();
			auto& execSys = registry.Get<BattleExecutionSystem>();

			ExecutionUnitRunTime& runtime = execRuntimeByEntity[subject];
			runtime.character = dataSys.GetCharacterID(subject);
			runtime.context   = AnimContext::Battle;

			const bool ok = execSys.BeginAction(subject, intent, runtime);
			(void)ok;
		});
	listenerIds.push_back(idCommited);

	// IntroReady -> UIFocus 해제
	BattleEventListenerId idIntroReady = eventBus.Subscribe(BattleBusEventType::IntroReady, [this](const BattleEvent& event)
		{
			auto& input = registry.Get<InputService>();
			input.SetFocus(FocusState::None);
			input.SetManualTime(0.f);
		});
	listenerIds.push_back(idIntroReady);
}

void BattleOrchestraSystem::UnwireSubscriptions()
{
	for (BattleEventListenerId id : listenerIds)
		eventBus.Unsubscribe(id);
	listenerIds.clear();
}

void BattleOrchestraSystem::PumpSessionEventsToBus()
{
	auto& sessionSys = registry.Get<BattleSessionSystem>();
	for (const auto& event : sessionSys.PeekEvent())
	{
		BattleEvent busEvent{};
		busEvent.subjectEntity = invalidEntity;
		busEvent.subjectTeam = BattleTeam::Neutral;

		switch (event.type)
		{
		case BattleSessionEventType::IntroReady:
			busEvent.eventType = BattleBusEventType::IntroReady;
			busEvent.note = L"session intro ready";
			eventBus.Publish(busEvent);
			break;

		case BattleSessionEventType::SessionBegan:
			busEvent.eventType = BattleBusEventType::SessionBegan;
			busEvent.note = L"session began";
			eventBus.Publish(busEvent);
			break;

		case BattleSessionEventType::SessionActivated:
		{
			busEvent.eventType = BattleBusEventType::SessionActivated;
			busEvent.note    = L"session active";
			EventPayload_SessionPhase phase{};
			phase.newPhase   = BattlePhase::Active;
			busEvent.payload = phase;
			eventBus.Publish(busEvent);
			break;
		}

		case BattleSessionEventType::SessionResultDecided:
			busEvent.eventType = BattleBusEventType::SessionResultDecided;
			busEvent.note = L"result decided";
			eventBus.Publish(busEvent);
			break;

		case BattleSessionEventType::SessionEnded:
			busEvent.eventType = BattleBusEventType::SessionEnded;
			busEvent.note = L"session ended";
			eventBus.Publish(busEvent);
			break;

		default: break;
		}
	}
	sessionSys.ClearEvents();
}

void BattleOrchestraSystem::PumpTimelineEventsToBus()
{
	auto& timelineSys = registry.Get<BattleTimelineSystem>();

	for (const auto& timelineEvent : timelineSys.PeekEvents())
	{
		BattleEvent busEvent{};
		busEvent.subjectEntity = timelineEvent.subjectEntity;
		busEvent.subjectTeam   = timelineEvent.subjectTeam;
		busEvent.note          = timelineEvent.note;

		switch (timelineEvent.eventType)
		{
		case BattleTimelineEventType::FullGauge:
			busEvent.eventType = BattleBusEventType::TimelineFullGauge;
			eventBus.Publish(busEvent);
			break;

		case BattleTimelineEventType::ActionCommitted:
		{
			busEvent.eventType = BattleBusEventType::TimelineActionCommitted;

			// Intent 채워서 -> Payload 전달
			TimelineActionIntent intent{};
			if (TryFillIntentFromTimeline(timelineEvent.subjectEntity, intent))
			{
				EventPayload_ActionIntent payload{};
				payload.intent   = intent;
				busEvent.payload = payload;
			}
			eventBus.Publish(busEvent);
			break;
		}

		case BattleTimelineEventType::ActionFinished:
			busEvent.eventType = BattleBusEventType::TimelineActionFinished;
			eventBus.Publish(busEvent);
			break;

		case BattleTimelineEventType::TimelinePaused:
			busEvent.eventType = BattleBusEventType::TimelinePaused;
			eventBus.Publish(busEvent);
			break;

		case BattleTimelineEventType::TimelineResumed:
			busEvent.eventType = BattleBusEventType::TimelineResumed;
			eventBus.Publish(busEvent);
			break;

		case BattleTimelineEventType::ApChanged:
		{
			busEvent.eventType = BattleBusEventType::TimelineApChanged;
			
			int curAp = 0, maxAp = 0;
			TryFillApSnapShot(timelineEvent.subjectEntity, curAp, maxAp);

			EventPayload_ApChanged ap{};
			ap.deltaAp       = timelineEvent.deltaAp;
			ap.curAp         = curAp;
			ap.maxAp         = maxAp;
			busEvent.payload = ap;

			eventBus.Publish(busEvent);
			break;
		}
		default: break;
		}
	}
	timelineSys.ClearEvents();
}

void BattleOrchestraSystem::TickExecutions(float dt)
{
	if (execRuntimeByEntity.empty()) return;

	auto& execSys = registry.Get<BattleExecutionSystem>();

	for (auto& pair : execRuntimeByEntity)
	{
		const EntityID entity = pair.first;
		ExecutionUnitRunTime& runtime = pair.second;

		if (runtime.cursor.isActive)
			execSys.Tick(entity, dt, runtime);
	}
}

bool BattleOrchestraSystem::TryFillIntentFromTimeline(EntityID entity, TimelineActionIntent& outIntent) const
{
	auto& timelineSys = registry.Get<BattleTimelineSystem>();
	const TimelineUnitState* unitState{};
	BattleTeam team{};
	int slotIdx{};
	if (!timelineSys.TryGetUnitStateByEntity(entity, team, slotIdx, unitState) || !unitState) return false;

	outIntent = unitState->activeIntent;
	return (outIntent.battleCmd != BattleCommand::None);
}

bool BattleOrchestraSystem::TryFillApSnapShot(EntityID entity, int& outCurAp, int& outMaxAp) const
{
	auto& timelineSys = registry.Get<BattleTimelineSystem>();
	const TimelineUnitState* unitState{};
	BattleTeam team{};
	int slotIdx{};
	if (!timelineSys.TryGetUnitStateByEntity(entity, team, slotIdx, unitState) || !unitState) return false;

	outCurAp = unitState->ap.curAp;
	outMaxAp = unitState->ap.maxAp;
	return true;
}