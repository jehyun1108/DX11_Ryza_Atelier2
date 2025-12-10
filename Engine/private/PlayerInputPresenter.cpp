#include "Enginepch.h"
#include "PlayerInputPresenter.h"
#include "BattleHUDPresenter.h"

void PlayerInputPresenter::OnBoot()
{
	uiRegistry   = &registry.Get<UIRegistry>();
	uiAnimSys    = &registry.Get<UIAnimSystem>();
	timelineSys  = &registry.Get<BattleTimelineSystem>();
	eventBus     = &registry.Get<BattleEventBus>();
	ctrlSys      = &registry.Get<BattleControllerSystem>();
	uiSys        = &registry.Get<UISystem>();
	dataSys      = &registry.Get<CharacterDataSystem>();

	SkillViewSet ryza{};
	ryza.slots[0] = { L"ryza_skill_1", L"샤이닝 블라스트" };
	ryza.slots[1] = { L"ryza_skill_2", L"월윈드" };
	ryza.slots[2] = { L"ryza_skill_3", L"블레이즈 러시" };
	ryza.slots[3] = { L"ryza_skill_4", L"크리스털라이즈 오라" };
	skillViews[CharacterID::Ryza] = ryza;

	SkillViewSet pat{};
	pat.slots[0] = { L"patricia_skill_1", L"홍염참" };
	pat.slots[1] = { L"patricia_skill_2", L"퀵 드로" };
	pat.slots[2] = { L"patricia_skill_3", L"블레이드 댄스" };
	pat.slots[3] = { L"patricia_skill_4", L"탈론 재퍼" };
	skillViews[CharacterID::Patricia] = pat;

	SkillViewSet kla{};
	kla.slots[0] = { L"klaudia_skill_1", L"게일 스파이크 +" };
	kla.slots[1] = { L"klaudia_skill_2", L"인피니티 애로" };
	kla.slots[2] = { L"klaudia_skill_3", L"프로즌 릴리" };
	kla.slots[3] = { L"klaudia_skill_4", L"정령의 무도" };
	skillViews[CharacterID::Klaudia] = kla;
}

void PlayerInputPresenter::Enter()
{
	EnsureSlots();
	WireSubs();

	SetUpPrimaryView();

	rt.leader = timelineSys->GetLeader();
	rt.ready = ResolveReady();

	rt.primaryEverReady = rt.ready;

	for (int i = 0; i < 4; ++i)
	{
		const wstring& key = cfg.highlightKeys[(size_t)i];
		uiAnimSys->SetOpacity(key, 0.f);
		uiAnimSys->SetScale(key, 1.f, 1.f); 
		rt.highlightT[(size_t)i] = 0.f;
	}

	ApplyVisibility(rt.ready);
	ApplyEnable();
}

void PlayerInputPresenter::Tick(float dt)
{
	const EntityID curLeader = timelineSys->GetLeader();
	if (curLeader != rt.leader)
		OnLeaderChanged();

	const bool readyNow = ResolveReady();
	if (readyNow)
		rt.primaryEverReady = true;

	rt.ready = readyNow;

	ApplyVisibility(rt.ready);
	ApplyEnable();

	const ControllerRuntime& ctrlRt = ctrlSys->GetRuntime();
	const CommandMenuPage page = ctrlRt.menu.page;

	CharacterID curChar = CharacterID::Unknown;
	if (rt.leader != invalidEntity)
		curChar = dataSys->GetCharacterID(rt.leader);

	if (page != lastPage || curChar != lastChar)
	{
		lastPage = page;
		lastChar = curChar;
		RefreshViewForLeader(page, curChar);
	}

	UpdateHighlights(dt);
}

void PlayerInputPresenter::Exit()
{
	UnWireSubs();
	for (int i = 0; i < 4; ++i) EnableSlot(i, false);
	rt = {};
}

void PlayerInputPresenter::EnsureSlots()
{
	for (int i = 0; i < 4; ++i)
	{
		const auto& slot = cfg.keys.slots[(size_t)i];
		uiRegistry->Ensure(slot.back);
		uiRegistry->Ensure(slot.outline);
		uiRegistry->Ensure(slot.icon);
		uiRegistry->Ensure(slot.key);
		uiRegistry->EnsureClone(L"battle_input_label", slot.label);

		const _float2& pos = cfg.labelPos[(size_t)i];
		uiRegistry->SetLocalPos(slot.label, pos.x, pos.y);
		const wstring& hlKey = cfg.highlightKeys[(size_t)i];
		uiRegistry->Ensure(hlKey);
		EnableSlot(i, true);
	}
}

void PlayerInputPresenter::ApplyVisibility(bool ready)
{
	const ControllerRuntime& ctrlRt = ctrlSys->GetRuntime();
	const CommandMenuPage page = ctrlRt.menu.page;
	const bool isExecuting = ctrlRt.isExecuting;

	const bool  defendOk = DefendAllowed();
	const float dim = cfg.anim.dimA;

	if (page == CommandMenuPage::Primary)
	{
		if (!rt.primaryEverReady)
		{
			SetAlphaSlot(0, defendOk ? 1.f : dim);
			SetAlphaSlot(1, 0.f);
			SetAlphaSlot(2, 0.f);
			SetAlphaSlot(3, 0.f);
			return;
		}
		if (!ready)
		{
			SetAlphaSlot(0, defendOk ? 1.f : dim);
			SetAlphaSlot(1, dim);
			SetAlphaSlot(2, dim);
			SetAlphaSlot(3, dim);
		}
		else
		{
			SetAlphaSlot(0, defendOk ? 1.f : dim);
			SetAlphaSlot(1, 1.f);
			SetAlphaSlot(2, 1.f);
			SetAlphaSlot(3, 1.f);
		}
	}
	else if (page == CommandMenuPage::Skill)
	{
		if (isExecuting)
		{
			SetAlphaSlot(0, dim);
			SetAlphaSlot(1, dim);
			SetAlphaSlot(2, dim);
			SetAlphaSlot(3, dim);
		}
		else if (ready)
		{
			SetAlphaSlot(0, 1.f);
			SetAlphaSlot(1, 1.f);
			SetAlphaSlot(2, 1.f);
			SetAlphaSlot(3, 1.f);
		}
		else
		{
			SetAlphaSlot(0, dim);
			SetAlphaSlot(1, dim);
			SetAlphaSlot(2, dim);
			SetAlphaSlot(3, dim);
		}
	}
	else
	{
		SetAlphaSlot(0, 0.f);
		SetAlphaSlot(1, 0.f);
		SetAlphaSlot(2, 0.f);
		SetAlphaSlot(3, 0.f);
	}
}

void PlayerInputPresenter::ApplyEnable()
{
	if (!DefendAllowed())
		SetAlphaSlot(0, cfg.anim.dimA);

	if (rt.anyKeyHeld)
	{
		const float dim = cfg.anim.dimA;
		SetAlphaSlot(0, dim);
		if (rt.ready)
		{
			SetAlphaSlot(1, dim);
			SetAlphaSlot(2, dim);
			SetAlphaSlot(3, dim);
		}
	}
}

void PlayerInputPresenter::WireSubs()
{
	if (wired) return;

	listenerIds.push_back(eventBus->Subscribe(BattleBusEventType::LeaderChanged,           [&](const BattleEvent&) { OnLeaderChanged();   }));
	listenerIds.push_back(eventBus->Subscribe(BattleBusEventType::TimelineFullGauge,       [&](const BattleEvent&) { OnFullGauge();       }));
	listenerIds.push_back(eventBus->Subscribe(BattleBusEventType::TimelineApChanged,       [&](const BattleEvent&) { OnApChanged();       }));
	listenerIds.push_back(eventBus->Subscribe(BattleBusEventType::TimelineActionCommitted, [&](const BattleEvent&) { OnActionCommitted(); }));
	listenerIds.push_back(eventBus->Subscribe(BattleBusEventType::TimelineActionFinished,  [&](const BattleEvent&) { OnActionFinished();  }));

	wired = true;
}

void PlayerInputPresenter::UnWireSubs()
{
	if (!wired) return;
	for (auto id : listenerIds) eventBus->Unsubscribe(id);
	listenerIds.clear();
	wired = false;
}

void PlayerInputPresenter::OnLeaderChanged()
{
	rt.leader = timelineSys->GetLeader();
	if (rt.leader == invalidEntity)
	{
		for (int i = 0; i < 4; ++i) SetAlphaSlot(i, 0.f);
		uiAnimSys->Tick(0.f);
		return;
	}

	rt.ready = ResolveReady();
	ApplyVisibility(rt.ready);
	ApplyEnable();
}

void PlayerInputPresenter::OnFullGauge()
{
	rt.ready = ResolveReady();
	ApplyVisibility(rt.ready);
	ApplyEnable();
}

void PlayerInputPresenter::OnApChanged()
{
	if (rt.leader == invalidEntity) return;

	int cur{}, max{};
	timelineSys->GetApSnapshot(rt.leader, cur, max);

	rt.curAp = cur;
	rt.maxAp = max;

	ApplyEnable();
}

void PlayerInputPresenter::OnActionFinished()
{
	rt.ready = ResolveReady();
	ApplyVisibility(rt.ready);
	ApplyEnable();
}

bool PlayerInputPresenter::ResolveReady() const
{
	const EntityID leader = timelineSys->GetLeader();
	if (leader == invalidEntity) return false;
	return timelineSys->IsUnitReadyToAct(leader);
}

bool PlayerInputPresenter::DefendAllowed() const
{
	const EntityID leader = timelineSys->GetLeader();
	if (leader == invalidEntity) return false;
	return timelineSys->IsDefendAllowed(leader);
}

void PlayerInputPresenter::SetAlphaSlot(int idx, float a)
{
	const auto& slot = cfg.keys.slots[(size_t)idx];
	uiAnimSys->SetOpacity(slot.back, a);
	uiAnimSys->SetOpacity(slot.outline, a);
	uiAnimSys->SetOpacity(slot.icon, a);
	uiAnimSys->SetOpacity(slot.key, a);
	uiAnimSys->SetOpacity(slot.label, a);
}

void PlayerInputPresenter::EnableSlot(int idx, bool on)
{
	const auto& slot = cfg.keys.slots[(size_t)idx];
	uiRegistry->SetEnabled(slot.back, on);
	uiRegistry->SetEnabled(slot.outline, on);
	uiRegistry->SetEnabled(slot.icon, on);
	uiRegistry->SetEnabled(slot.key, on);
	uiRegistry->SetEnabled(slot.label, on);
}

void PlayerInputPresenter::RefreshViewForLeader(CommandMenuPage page, CharacterID characterId)
{
	if (page == CommandMenuPage::Primary)
		SetUpPrimaryView();
	else if (page == CommandMenuPage::Skill)
		SetUpSkillView(characterId);
}

void PlayerInputPresenter::SetUpPrimaryView()
{
	static const wchar_t* labels[4] = {L"방어",L"공격",L"아이템",L"도망",};
	for (int i = 0; i < 4; ++i)
	{
		const auto& slot = cfg.keys.slots[(size_t)i];
		uiSys->SetText(slot.label, labels[i]);
	}

	static const wchar_t* primaryIconTex[4] = {L"defend_icon",L"attack_icon",L"itemrush_icon",L"flee_icon", };
	for (int i = 0; i < 4; ++i)
	{
		const auto& slot = cfg.keys.slots[(size_t)i];
		uiRegistry->SetWidgetTexture(slot.icon, primaryIconTex[(size_t)i]);
	}
}

void PlayerInputPresenter::SetUpSkillView(CharacterID characterId)
{
	const SkillViewSet& view = skillViews.at(characterId);

	for (int i = 0; i < 4; ++i)
	{
		const auto& slot = cfg.keys.slots[(size_t)i];
		const SkillSlotView& sv = view.slots[(size_t)i];

		uiRegistry->SetWidgetTexture(slot.icon, sv.iconTexKey);
		uiSys->SetText(slot.label, sv.labelText);
	}
}

void PlayerInputPresenter::UpdateHighlights(float dt)
{
	const ControllerRuntime& ctrlRt = ctrlSys->GetRuntime();
	const CommandMenuPage page = ctrlRt.menu.page;
	const bool isExecuting = ctrlRt.isExecuting;

	const bool defendOk = DefendAllowed();
	const bool ready = rt.ready;

	for (int i = 0; i < 4; ++i)
	{
		const wstring& key = cfg.highlightKeys[(size_t)i];

		bool highlight = false;

		if (page == CommandMenuPage::Primary)
		{
			if (!isExecuting)
			{
				if (i == 0)
					highlight = defendOk;
				else
					highlight = ready;
			}
		}
		else if (page == CommandMenuPage::Skill)
		{
			if (!isExecuting && ready)
				highlight = true;
		}

		if (!highlight)
		{
			uiAnimSys->SetOpacity(key, 0.f);
			uiAnimSys->SetScale(key, 1.f, 1.f);
			rt.highlightT[(size_t)i] = 0.f;
			continue;
		}

		float& t = rt.highlightT[(size_t)i];
		t += dt;

		const float period = 1.f;
		float localT = fmodf(t, period);   
		float norm = localT / period;

		float scale = 1.0f + 0.5f * norm;  
		float alpha = 1.0f - norm;         

		uiAnimSys->SetScale(key, scale, scale);
		uiAnimSys->SetOpacity(key, alpha);
	}
}
