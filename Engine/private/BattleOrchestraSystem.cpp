#include "Enginepch.h"

void BattleOrchestraSystem::OnBoot()
{
	eventBus       = &registry.Get<BattleEventBus>();
	uiOrchestrator = &registry.Get<BattleUIOrchestrator>();
	input          = &registry.Get<InputService>();
	camDirector    = &registry.Get<BattleCameraDirector>();
	camReg         = &registry.Get<CamRegistry>();
	camSys         = &registry.Get<CameraSystem>();
	sessionSys     = &registry.Get<BattleSessionSystem>();
	introSys       = &registry.Get<BattleIntroSystem>();
	ctrlSys        = &registry.Get<BattleControllerSystem>();
	timelineSys    = &registry.Get<BattleTimelineSystem>();
	aiCtrlSys      = &registry.Get<BattleAIControllerSystem>();
	execSys        = &registry.Get<BattleExecutionSystem>();
	tfSys          = &registry.Get<TransformSystem>();
	animator       = &registry.Get<AnimatorSystem>();
	moveSys        = &registry.Get<MoveStateSystem>();
	dataSys        = &registry.Get<CharacterDataSystem>();
	targetSys      = &registry.Get<BattleTargetSystem>();

	assert(eventBus && uiOrchestrator && input && camDirector && camReg && camSys && sessionSys && introSys && ctrlSys && timelineSys && aiCtrlSys && execSys && tfSys && animator && moveSys && dataSys && targetSys);
}

void BattleOrchestraSystem::Enter()
{
	input->SetContext(InputContext::Battle);
	input->SetFocus(FocusState::UI);
	input->SetManualTime(0.f);

	eventBus->ReserveQueue(256);
	WireSubscriptions();
// ==============================================================
	const Handle mainCam = camSys->GetMainCamHandle();
	camDirector->BindCam(mainCam);
	camSys->ClearTarget(mainCam);
	camReg->BindDirector();
	camReg->RegisterDefaults();
	WireCameraSubscriptions();
	camReg->SpawnDefaultToFollow();
// =================================================================
	uiOrchestrator->Enter();
}

void BattleOrchestraSystem::Update(float dt)
{
	sessionSys->Update(dt);
	introSys->Update(dt);
	PumpSessionEventsToBus();

	if (sessionSys->GetPhase() == BattlePhase::Active)
	{
		ctrlSys->Update(sessionSys->GetLeader(), dt);
		aiCtrlSys->Update(dt);
		timelineSys->Tick(dt);
		execSys->Tick(dt);
		PumpTimelineEventsToBus();
	}
	if (camDirector) camDirector->Tick(dt);

	uiOrchestrator->Tick(dt);
	eventBus->DispatchAll();
	eventBus->ClearQueue();
}

void BattleOrchestraSystem::Exit()
{
	uiOrchestrator->Exit();
	UnwireCameraSubscriptions();
	UnwireSubscriptions();

	if (camReg) 
		camReg->ClearAll();

	input->SetFocus(FocusState::None);
}

bool BattleOrchestraSystem::BeginBattle(const BattleStartParams& Inparams)
{
	auto  params      = Inparams;

	FormationParams fParams;
	fParams.ringRadius       = params.ringRadius;     
	fParams.allyStartDeg     = params.startAngleDeg;    
	fParams.enemyStartDeg    = params.startAngleDeg + 180.f;

	sessionSys->BeginSession(params.allies, params.enemies, params.centerWorld, fParams, params.sessionConfig);
	targetSys->Init();

	auto createIntroFor = [&](EntityID entity) 
		{
			if (entity == invalidEntity)  return;

			MoveState* move = moveSys->GetByOwner(entity);
			if (!move) return;

			Handle tfHandle = move->tfHandle;
			Handle animHandle{};
			animator->GetByOwner(entity, &animHandle);
			if (!animHandle.IsValid()) return;

			AnimProfile profile = dataSys->ResolveProfile(entity, AnimContext::Battle);
			introSys->Create(entity, animHandle, tfHandle, profile);
		};

	for (int i = 0; i < params.allies.memberCount; ++i)  createIntroFor(params.allies.members[i]);
	for (int i = 0; i < params.enemies.memberCount; ++i) createIntroFor(params.enemies.members[i]);

	if (const BattleSessionState* state = sessionSys->TryGetState())
	{
		BattleTimelineConfig timelineConfig{};
		timelineSys->InitSession(*state, timelineConfig);
	}
// ======================================================================================================
	return true;
}

void BattleOrchestraSystem::WireSubscriptions()
{
	listenerIds.clear();

	// TimelineFullGauge → Controller 턴 시작
	listenerIds.push_back( eventBus->Subscribe(BattleBusEventType::TimelineFullGauge, [&](const BattleEvent&) { ctrlSys->OnGaugeBecameFull(); }));

	// TimelineActionCommitted → ExecutionSystem 시작
	listenerIds.push_back( eventBus->Subscribe(BattleBusEventType::TimelineActionCommitted,
			[&](const BattleEvent& event)
			{
				const EntityID subject = event.subjectEntity;
				if (subject == invalidEntity) return;

				TimelineActionIntent intent{};
				if (!TryFillIntentFromTimeline(subject, intent)) return;
				execSys->BeginAction(subject, intent);
			})
	);

	// TimelineActionFinished → Controller 턴 정리
	listenerIds.push_back( eventBus->Subscribe(BattleBusEventType::TimelineActionFinished, 
		[&](const BattleEvent& e){ ctrlSys->OnActionExecutionFinished(TimelineActionIntent{});}));

	// Active 진입시
	listenerIds.push_back(eventBus->Subscribe(BattleBusEventType::SessionActivated, [&](const BattleEvent& e) {targetSys->Init(); }));

	// IntroReady → UI Focus 해제
	listenerIds.push_back(eventBus->Subscribe(BattleBusEventType::IntroReady, [&](const BattleEvent&) {input->SetFocus(FocusState::None);input->SetManualTime(0.f);}));

	// UnitDowned
	listenerIds.push_back(eventBus->Subscribe(BattleBusEventType::UnitDowned, [&](const BattleEvent& e) { targetSys->OnUnitDowned(e.subjectEntity); }));

	// LeaderSwitch
}

void BattleOrchestraSystem::UnwireSubscriptions()
{
	for (auto id : listenerIds)
		eventBus->Unsubscribe(id);
	listenerIds.clear();
}

void BattleOrchestraSystem::WireCameraSubscriptions()
{
	listenerIds.push_back(eventBus->Subscribe(BattleBusEventType::SessionActivated, [&](const BattleEvent&) { if (camReg) camReg->SpawnIntro(); }));
	listenerIds.push_back(eventBus->Subscribe(BattleBusEventType::TimelineActionFinished, [&](const BattleEvent&) { if (camReg) camReg->KillRecent(0.7f); }));
}

void BattleOrchestraSystem::UnwireCameraSubscriptions()
{

}

void BattleOrchestraSystem::PumpSessionEventsToBus()
{
	for (const auto& event : sessionSys->PeekEvent())
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
			eventBus->Publish(busEvent);
	}
	sessionSys->ClearEvents();
}

void BattleOrchestraSystem::PumpTimelineEventsToBus()
{
	for (const auto& timelineEvent : timelineSys->PeekEvents())
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
			eventBus->Publish(busEvent);
	}
	timelineSys->ClearEvents();
}

bool BattleOrchestraSystem::TryFillIntentFromTimeline(EntityID entity, TimelineActionIntent& outIntent) const
{
	const TimelineUnitState* unitState{};
	BattleTeam team{};
	int slotIdx{};
	if (!timelineSys->TryGetUnitStateByEntity(entity, team, slotIdx, unitState) || !unitState) return false;

	outIntent = unitState->activeIntent;
	return (outIntent.battleCmd != BattleCommand::None);
}

bool BattleOrchestraSystem::TryFillApSnapShot(EntityID entity, int& outCurAp, int& outMaxAp) const
{
	const TimelineUnitState* unitState{};
	BattleTeam team{};
	int slotIdx{};
	if (!timelineSys->TryGetUnitStateByEntity(entity, team, slotIdx, unitState) || !unitState) return false;

	outCurAp = unitState->ap.curAp;
	outMaxAp = unitState->ap.maxAp;
	return true;
}