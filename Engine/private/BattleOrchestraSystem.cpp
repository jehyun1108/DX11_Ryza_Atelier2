#include "Enginepch.h"

void BattleOrchestraSystem::Enter()
{
	uiOrchestrator = make_unique<BattleUIOrchestrator>(registry);

	auto& input = registry.Get<InputService>();
	input.SetContext(InputContext::Battle);
	input.SetFocus(FocusState::UI);
	input.SetManualTime(0.f);

	eventBus.ReserveQueue(256);
	WireSubscriptions();
// ==============================================================
	camDirector = make_unique<BattleCameraDirector>(registry);
	camReg      = make_unique<CamRegistry>(registry);

	auto& camSys = registry.Get<CameraSystem>();
	const Handle mainCam = camSys.GetMainCamHandle();
	camDirector->BindCam(mainCam);
	camSys.ClearTarget(mainCam);

	camReg->BindDirector(*camDirector);
	camReg->RegisterDefaults();

	WireCameraSubscriptions();
	camReg->SpawnDefaultToFollow();
// =================================================================
	uiOrchestrator->Enter();
}

void BattleOrchestraSystem::Update(float dt)
{
	auto& sessionSys   = registry.Get<BattleSessionSystem>();
	auto& intro        = registry.Get<BattleIntroSystem>();
	auto& controller   = registry.Get<BattleControllerSystem>();
	auto& timelineSys  = registry.Get<BattleTimelineSystem>();
	auto& aiController = registry.Get<BattleAIControllerSystem>();
	auto& execSys      = registry.Get<BattleExecutionSystem>();

	sessionSys.Update(dt);
	intro.Update(dt);
	PumpSessionEventsToBus();

	if (sessionSys.GetPhase() == BattlePhase::Active)
	{
		controller.Update(sessionSys.GetLeader(), dt);
		aiController.Update(dt);
		timelineSys.Tick(dt);
		execSys.Tick(dt);
		PumpTimelineEventsToBus();
	}
	if (camDirector) camDirector->Tick(dt);

	uiOrchestrator->Tick(dt);
	eventBus.DispatchAll();
	eventBus.ClearQueue();
}

void BattleOrchestraSystem::Exit()
{
	uiOrchestrator->Exit();
	UnwireCameraSubscriptions();
	UnwireSubscriptions();

	if (camReg) camReg->ClearAll();
	camReg.reset();
	camDirector.reset();

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
	auto& targetSys   = registry.Get<BattleTargetSystem>();

	FormationParams fParams;
	fParams.ringRadius       = params.ringRadius;     
	fParams.allyStartDeg     = params.startAngleDeg;    
	fParams.enemyStartDeg    = params.startAngleDeg + 180.f;

	sessionSys.BeginSession(params.allies, params.enemies, params.centerWorld, fParams, params.sessionConfig);
	targetSys.Init();

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
// ======================================================================================================
	return true;
}

void BattleOrchestraSystem::WireSubscriptions()
{
	listenerIds.clear();

	auto& controller = registry.Get<BattleControllerSystem>();
	auto& execSys    = registry.Get<BattleExecutionSystem>();
	auto& input      = registry.Get<InputService>();
	auto& targetSys  = registry.Get<BattleTargetSystem>();

	// TimelineFullGauge → Controller 턴 시작
	listenerIds.push_back( eventBus.Subscribe(BattleBusEventType::TimelineFullGauge, [&](const BattleEvent&) { controller.OnGaugeBecameFull(); }));

	// TimelineActionCommitted → ExecutionSystem 시작
	listenerIds.push_back( eventBus.Subscribe(BattleBusEventType::TimelineActionCommitted,
			[&](const BattleEvent& event)
			{
				const EntityID subject = event.subjectEntity;
				if (subject == invalidEntity) return;

				TimelineActionIntent intent{};
				if (!TryFillIntentFromTimeline(subject, intent)) return;
				execSys.BeginAction(subject, intent);
			})
	);

	// TimelineActionFinished → Controller 턴 정리
	listenerIds.push_back(
		eventBus.Subscribe(BattleBusEventType::TimelineActionFinished, 
			[&](const BattleEvent& e){ controller.OnActionExecutionFinished(TimelineActionIntent{});}));

	// Active 진입시
	listenerIds.push_back(eventBus.Subscribe(BattleBusEventType::SessionActivated, [&](const BattleEvent& e) {targetSys.Init(); }));

	// IntroReady → UI Focus 해제
	listenerIds.push_back(eventBus.Subscribe(BattleBusEventType::IntroReady,
		[&](const BattleEvent&) {input.SetFocus(FocusState::None);input.SetManualTime(0.f);}));

	// UnitDowned
	listenerIds.push_back(eventBus.Subscribe(BattleBusEventType::UnitDowned, [&](const BattleEvent& e) {targetSys.OnUnitDowned(e.subjectEntity); }));

	// LeaderSwitch
}

void BattleOrchestraSystem::UnwireSubscriptions()
{
	for (auto id : listenerIds)
		eventBus.Unsubscribe(id);
	listenerIds.clear();
}

void BattleOrchestraSystem::WireCameraSubscriptions()
{
	listenerIds.push_back(eventBus.Subscribe(BattleBusEventType::SessionActivated, [&](const BattleEvent&) { if (camReg) camReg->SpawnIntro(); }));
	listenerIds.push_back(eventBus.Subscribe(BattleBusEventType::TimelineActionFinished, [&](const BattleEvent&) { if (camReg) camReg->KillRecent(0.7f); }));
}

void BattleOrchestraSystem::UnwireCameraSubscriptions()
{

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
			break;
		case BattleSessionEventType::SessionBegan:
			busEvent.eventType = BattleBusEventType::SessionBegan;
			break;
		case BattleSessionEventType::SessionActivated:
			busEvent.eventType = BattleBusEventType::SessionActivated;
			break;
		case BattleSessionEventType::SessionResultDecided:
			busEvent.eventType = BattleBusEventType::SessionResultDecided;
			break;
		case BattleSessionEventType::SessionEnded:
			busEvent.eventType = BattleBusEventType::SessionEnded;
			break;
		default: break;
		}

		if (busEvent.eventType != BattleBusEventType::None)
			eventBus.Publish(busEvent);
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
		busEvent.subjectTeam = timelineEvent.subjectTeam;

		switch (timelineEvent.eventType)
		{
		case BattleTimelineEventType::FullGauge:
			busEvent.eventType = BattleBusEventType::TimelineFullGauge;
			break;

		case BattleTimelineEventType::ActionCommitted:
			busEvent.eventType = BattleBusEventType::TimelineActionCommitted;
			break;

		case BattleTimelineEventType::ActionFinished:
			busEvent.eventType = BattleBusEventType::TimelineActionFinished;
			break;

		case BattleTimelineEventType::ApChanged:
		{
			busEvent.eventType = BattleBusEventType::TimelineApChanged;
			int curAp = 0, maxAp = 0;
			TryFillApSnapShot(timelineEvent.subjectEntity, curAp, maxAp);
			EventPayload_ApChanged ap{};
			ap.deltaAp = timelineEvent.deltaAp;
			ap.curAp   = curAp;
			ap.maxAp   = maxAp;
			busEvent.payload = ap;
			break;
		}
		default: break;
		}

		if (busEvent.eventType != BattleBusEventType::None)
			eventBus.Publish(busEvent);
	}
	timelineSys.ClearEvents();
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