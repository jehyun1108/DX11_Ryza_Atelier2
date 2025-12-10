#include "Enginepch.h"
#include "BattleAttributeSystem.h"
#include "ScreenFadeSystem.h"
#include "BattleRewardPresenter.h"
#include "SoundSystem.h"
#include "BattleDamagePresenter.h"

void BattleOrchestraSystem::OnBoot()
{
	eventBus        = &registry.Get<BattleEventBus>();
	uiOrchestrator  = &registry.Get<BattleUIOrchestrator>();
	input           = &registry.Get<InputService>();
	camDirector     = &registry.Get<BattleCameraDirector>();
	camReg          = &registry.Get<CamRegistry>();
	camSys          = &registry.Get<CameraSystem>();
	sessionSys      = &registry.Get<BattleSessionSystem>();
	introSys        = &registry.Get<BattleIntroSystem>();
	ctrlSys         = &registry.Get<BattleControllerSystem>();
	timelineSys     = &registry.Get<BattleTimelineSystem>();
	aiCtrlSys       = &registry.Get<BattleAIControllerSystem>();
	execSys         = &registry.Get<BattleExecutionSystem>();
	tfSys           = &registry.Get<TransformSystem>();
	animator        = &registry.Get<AnimatorSystem>();
	moveSys         = &registry.Get<MoveStateSystem>();
	dataSys         = &registry.Get<CharacterDataSystem>();
	targetSys       = &registry.Get<BattleTargetSystem>();
	tacticSys       = &registry.Get<BattleTacticSystem>();
	attributeSys    = &registry.Get<BattleAttributeSystem>();
	animSys         = &registry.Get<AnimDataSystem>();
	formSys         = &registry.Get<BattleFormationSystem>();
	fadeSys         = &registry.Get<ScreenFadeSystem>();
	entityMgr       = &registry.Get<EntityMgr>();
	rewardPresenter = &registry.Get<BattleRewardPresenter>();
	soundSys        = &registry.Get<SoundSystem>();
	dmgPresenter    = &registry.Get<BattleDamagePresenter>();
}

void BattleOrchestraSystem::Enter()
{
	victorySequenceStarted = false;

	input->SetContext(InputContext::Battle);
	input->SetFocus(FocusState::UI);
	input->SetManualTime(0.f);
	eventBus->ReserveQueue(256);
	WireSubs();
// ==============================================================
	const Handle mainCam = camSys->GetMainCamHandle();
	camDirector->BindCam(mainCam);
	camSys->ClearTarget(mainCam);
	WireCamSubs();
	camReg->SpawnDefaultToFollow();
	dmgPresenter->Wire();
// =================================================================
	tacticSys->Reset();
	uiOrchestrator->Enter();
}

void BattleOrchestraSystem::Update(float dt)
{
	BattlePhase phase = sessionSys->GetPhase();

	if (phase == BattlePhase::Active)
	{
		sessionSys->Update(dt);
		introSys->Update(dt);
		PumpSessionEventsToBus();

		ctrlSys->Update(dt);
		aiCtrlSys->Update(dt);
		timelineSys->Tick(dt);
		execSys->Tick(dt);
		attributeSys->Tick(dt);
		PumpTimelineEventsToBus();
		TickHitReacts(dt);
	}
	else
	{
		sessionSys->Update(dt);
		ctrlSys->Update(dt);
		introSys->Update(dt);
		PumpSessionEventsToBus();
	}

	if (phase == BattlePhase::Result)
		rewardPresenter->Tick(dt);

	{
		static    int   shownCombo  = 0;      
		static    float zeroHold    = 0.f;   
		constexpr float zeroHoldSec = 2.f; 
		int rawCombo = execSys->GetComboCount();

		if (rawCombo > 0)
		{
			shownCombo = rawCombo;
			zeroHold = 0.f;
		}
		else
		{
			if (shownCombo > 0)
			{
				zeroHold += dt;
				if (zeroHold >= zeroHoldSec)
				{
					shownCombo = 0;
					zeroHold = 0.f;
				}
			}
		}
		dmgPresenter->SetChain(shownCombo);
	}

	camDirector->Tick(dt);
	eventBus->DispatchAll();
	uiOrchestrator->Tick(dt);
}

void BattleOrchestraSystem::Exit()
{
	uiOrchestrator->Exit();
	attributeSys->EndSession();
	UnwireCamSubs();
	UnwireSubs();
	camReg->ClearAll();
	input->SetFocus(FocusState::None);

	dmgPresenter->UnWire();

	victorySequenceStarted = false;

	for (EntityID id : deadEntities)
		entityMgr->DestroyDeferred(id);
	deadEntities.clear();
}

bool BattleOrchestraSystem::BeginBattle(const BattleStartParams& Inparams)
{
	auto params = Inparams;

	FormationParams intro{};
	intro.ringRadius = params.ringRadius;
	intro.allyStartDeg = params.startAngleDeg;
	intro.enemyStartDeg = params.startAngleDeg + 180.f;
	intro.allySpanDeg = 120.f;
	intro.enemySpanDeg = 120.f;
	intro.backMeters = 250.f;
	intro.charRadiusMeters = 40.f;
	intro.padDeg = 4.f;
	intro.allyLeaderSlot = 0;

	FormationParams battle = intro;

	battle.ringRadius = params.ringRadius * 0.6f;
	battle.enemySpanDeg = 80.f;
	battle.allySpanDeg = 220.f;   
	battle.backMeters = 60.f;
	battle.allyLeaderSlot = 0;

	sessionSys->BeginSession(
		params.allies,
		params.enemies,
		params.centerWorld,
		intro,
		battle,
		params.sessionConfig);

	attributeSys->InitForSession(sessionSys->GetAllies(), sessionSys->GetEnemies());
	targetSys->Init();

	auto createIntroFor = [&](EntityID entity)
		{
			if (entity == 0u) return;

			MoveState* move = moveSys->GetByOwner(entity);
			Handle     tfHandle = move->tfHandle;

			Handle animHandle{};
			animator->GetByOwner(entity, &animHandle);

			AnimProfile profile = dataSys->ResolveProfile(entity, AnimContext::Battle);
			introSys->Create(entity, animHandle, tfHandle, profile);
		};

	for (int i = 0; i < params.allies.memberCount; ++i)
		createIntroFor(params.allies.members[i]);
	for (int i = 0; i < params.enemies.memberCount; ++i)
		createIntroFor(params.enemies.members[i]);

	const BattleSessionState state = sessionSys->GetState();
	BattleTimelineConfig     timelineConfig;
	timelineSys->InitSession(state, timelineConfig);
	return true;
}

void BattleOrchestraSystem::WireSubs()
{
	listenerIds.clear();

	listenerIds.push_back( eventBus->Subscribe(BattleBusEventType::TimelineFullGauge, [&](const BattleEvent&) { ctrlSys->OnGaugeBecameFull(); }));
	listenerIds.push_back(
		eventBus->Subscribe(
			BattleBusEventType::ResolveDamageApplied,
			[&](const BattleEvent& e)
			{
				const auto& dmg = get<EventPayload_Damage>(e.payload);
				const EntityID target = dmg.targetEntity;
				if (target == 0u)
					return;
				timelineSys->OnDamageApplied(dmg);
				PlayHitReaction(target, dmg);
			}));

	listenerIds.push_back(eventBus->Subscribe(BattleBusEventType::SessionActivated, [&](const BattleEvent& e) 
		{
			formSys->SetPhase(FormationPhase::Battle);
			targetSys->Init();
		}));
	listenerIds.push_back(eventBus->Subscribe(BattleBusEventType::IntroReady, [&](const BattleEvent&) {input->SetFocus(FocusState::None);input->SetManualTime(0.f);}));
	listenerIds.push_back(eventBus->Subscribe(BattleBusEventType::UnitDowned, [&](const BattleEvent& e) 
		{
			const EntityID target = e.subjectEntity;
			if (target == 0u)
				return;

			if (sessionSys->GetTeam(target) == BattleTeam::Enemy)
				soundSys->Play(L"010_enemydown");

			targetSys->OnUnitDowned(target);
			execSys->OnUnitRemoved(target);
			timelineSys->OnUnitRemoved(target);

			deadEntities.push_back(target);

			if (!victorySequenceStarted && IsLastEnemyDown())
			{
				victorySequenceStarted = true;
				sessionSys->SetPhase(BattlePhase::Result);

				vector<EntityID> victoryOrder;
				const BattleParty& allies = sessionSys->GetAllies();

				for (int i = 0; i < allies.memberCount; ++i)
				{
					EntityID ally = allies.members[i];
					if (ally == 0u) continue;

					float hpRatio = attributeSys->GetHpRatio01(ally);
					if (hpRatio > 0.f)
						victoryOrder.push_back(ally);
				}

				if (!victoryOrder.empty())
					rewardPresenter->BeginVictory(victoryOrder);
			}
		}));
}

void BattleOrchestraSystem::UnwireSubs()
{
	for (auto id : listenerIds) eventBus->Unsubscribe(id);
	listenerIds.clear();
}

void BattleOrchestraSystem::WireCamSubs()
{
	listenerIds.push_back(
		eventBus->Subscribe(
			BattleBusEventType::SessionBegan,
			[&](const BattleEvent&)
			{
				camReg->SpawnIntro();
			}));

	listenerIds.push_back(
		eventBus->Subscribe(
			BattleBusEventType::IntroReady,
			[&](const BattleEvent&)
			{

			}));

	listenerIds.push_back(
		eventBus->Subscribe(
			BattleBusEventType::TimelineActionFinished,
			[&](const BattleEvent&)
			{
				camReg->KillRecent(0.2f);
			}));
}

void BattleOrchestraSystem::PumpSessionEventsToBus()
{
	for (const auto& event : sessionSys->PeekEvent())
	{
		BattleEvent busEvent{};
		busEvent.subjectEntity = 0u;
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

void BattleOrchestraSystem::TickHitReacts(float dt)
{
	if (hitReacts.empty()) return;

	for (int i = (int)hitReacts.size() - 1; i >= 0; --i)
	{
		HitReactRuntime& hr = hitReacts[(size_t)i];
		hr.remaining -= dt;
		if (hr.remaining > 0.f) continue;

		CharacterID ch = dataSys->GetCharacterID(hr.entity);

		AnimKey idleKey = AnimKey::Battle_Idle;  
		const wstring& idleName = animSys->GetClipName(ch, AnimContext::Battle, idleKey);
		ClipTuning idleTuning = animSys->GetClipTuning(ch, AnimContext::Battle, idleKey);

		Handle animHandle = animator->Get(hr.entity);
		animator->CrossFade( animHandle, 0, 1, idleName, 0.1f, ANIMTYPE::LOOP, idleTuning.startNormalized, idleTuning.endNormalized);
		hitReacts.erase(hitReacts.begin() + i);
	}
}

bool BattleOrchestraSystem::TryFillIntentFromTimeline(EntityID entity, TimelineActionIntent& outIntent) const
{
	BattleTeam team{};
	int slot{};
	auto unit = timelineSys->GetUnitStateByEntity(entity, team, slot);

	outIntent = unit.activeIntent;
	assert(outIntent.battleCmd != BattleCommand::None);
	return true;
}

bool BattleOrchestraSystem::TryFillApSnapShot(EntityID entity, int& outCurAp, int& outMaxAp) const
{
	timelineSys->GetApSnapshot(entity, outCurAp, outMaxAp);
	return true;
}

void BattleOrchestraSystem::PlayHitReaction(EntityID target, const EventPayload_Damage& dmg)
{
	BattleTeam team{};
	int slotIndex{};
	const TimelineUnitState& unit = timelineSys->GetUnitStateByEntity(target, team, slotIndex);

	if (unit.motionState != TimelineMotionState::Queued) return;
	if (execSys->IsActing(target)) return;

	const ControllerRuntime& ctrlRt = ctrlSys->GetRuntime();
	const bool isLeader = (target == ctrlRt.leaderEntity);

	if (isLeader && ctrlRt.isExecuting)
		return;

	const bool isDefendingHold = ctrlRt.isDefendingHold;

	CharacterID ch = dataSys->GetCharacterID(target);

	float volume = isLeader ? 0.5f : 0.3f;

	switch (ch)
	{
	case CharacterID::Ryza:     soundSys->Play(L"ryza_0", volume); break;
	case CharacterID::Klaudia:  soundSys->Play(L"klaudia_40", volume); break;
	case CharacterID::Patricia: soundSys->Play(L"patricia_31", volume); break;
	}

	AnimKey key = AnimKey::Battle_Hit;
	if (isLeader && isDefendingHold)
		key = AnimKey::Battle_Defend_Success;

	const wstring& clipName = animSys->GetClipName(ch, AnimContext::Battle, key);
	Handle animHandle = animator->Get(target);
	ClipTuning tuning = animSys->GetClipTuning(ch, AnimContext::Battle, key);

	animator->CrossFade(
		animHandle, 0, 1, clipName, 0.05f,
		ANIMTYPE::ONCE, tuning.startNormalized, tuning.endNormalized);

	float clipDur = animator->GetClipDuration(animHandle, clipName);
	float normSpan = tuning.endNormalized - tuning.startNormalized;
	if (normSpan <= 0.f) normSpan = 1.f;

	float speed = (tuning.playbackSpeed != 0.f) ? tuning.playbackSpeed : 1.f;
	float playSec = (clipDur * normSpan) / speed;

	HitReactRuntime hr{};
	hr.entity = target;
	hr.remaining = playSec;
	hitReacts.push_back(hr);
}

bool BattleOrchestraSystem::IsLastEnemyDown() const
{
	const BattleEnemies& enemies = sessionSys->GetEnemies();

	bool anyEnemy = false;

	for (int i = 0; i < enemies.memberCount; ++i)
	{
		EntityID e = enemies.members[i];
		if (e == 0u)
			continue;

		anyEnemy = true;

		float hpRatio = attributeSys->GetHpRatio01(e);
		if (hpRatio > 0.f)
			return false; 
	}
	return anyEnemy;
}