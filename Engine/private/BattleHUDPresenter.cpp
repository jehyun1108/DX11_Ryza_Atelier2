#include "Enginepch.h"
#include "BattleHUDPresenter.h"
#include "BattleAttributeSystem.h"

static inline float TacticFill(int pips) { return Utility::Saturate(pips / 5.f); }
static inline float BlinkAlpha(const TacticBlink& b) 
{
	float s = 0.5f + 0.5f * sinf(XM_2PI * b.freq * b.t);
	return b.minA + (b.maxA - b.minA) * s;
}
static inline void SampleShake(const UIShakeTrack& tr, float& outX, float& outY)
{
	const float TWO_PI = 6.2831853f;
	const float s = TWO_PI * tr.freq * tr.t;          
	const float t01 = (tr.dur > 0.f) ? min(tr.t / tr.dur, 1.f) : 1.f;
	const float hann = 0.5f * (1.f - cosf(TWO_PI * t01));
	const float env = hann;
	outX = tr.ampX * sinf(s + tr.phaseX) * env;
	outY = tr.ampY * sinf(s + tr.phaseY) * env;
}
// =======================================================================================================================================
void BattleHUDPresenter::OnBoot()
{
	uiRegistry   = &registry.Get<UIRegistry>();
	uiAnimSys    = &registry.Get<UIAnimSystem>();
	dataSys      = &registry.Get<CharacterDataSystem>();
	timelineSys  = &registry.Get<BattleTimelineSystem>();
	sessionSys   = &registry.Get<BattleSessionSystem>();
	eventBus     = &registry.Get<BattleEventBus>();
	tacticSys    = &registry.Get<BattleTacticSystem>();
	attributeSys = &registry.Get<BattleAttributeSystem>();
}

void BattleHUDPresenter::Enter()
{
	EnsureInstances();
	EnsureBars();
	EnsureTacticBars();
	WireSubs();

	RefreshLeaderAndParty();
	ApplyLeaderPortrait();
	ApplyPartyPortraits();

	SyncAllImmediate();
	const auto& state = tacticSys->GetState();
	ApplyTacticFromState(state.level, state.pips, state.maxBlink);
}

void BattleHUDPresenter::Tick(float dt)
{
	const EntityID curLeader = timelineSys->GetLeader();
	if (curLeader != rt.leader)
	{
		RefreshLeaderAndParty();
		ApplyLeaderPortrait();
		ApplyPartyPortraits();
		SyncAllImmediate();
		UpdateTexts();
	}
	UpdateHPFront();
	UpdateStunFront();
	UpdateTexts();

	TickTacticReveal(dt);
	TickTacticBlink(dt);
	TickLevelUpBanner(dt);
	TickPinch(dt);
	TickDamageGlow(dt);
}

void BattleHUDPresenter::EnsureInstances()
{
	uiRegistry->Ensure(config.keys.leaderPortrait);
	for (size_t i = 0; i < 2; ++i)
		uiRegistry->Ensure(config.keys.partyPortrait[i]);

	uiRegistry->SetLocalPos(config.keys.leaderPortrait, config.layout.leaderPos.x, config.layout.leaderPos.y);
	uiAnimSys->SetScale(config.keys.leaderPortrait, config.scales.leaderScale, config.scales.leaderScale);

	uiRegistry->SetLocalPos(config.keys.partyPortrait[0], config.layout.partyPos[0].x, config.layout.partyPos[0].y);
	uiRegistry->SetLocalPos(config.keys.partyPortrait[1], config.layout.partyPos[1].x, config.layout.partyPos[1].y);
	uiAnimSys->SetScale(config.keys.partyPortrait[0], config.scales.partyScale, config.scales.partyScale);
	uiAnimSys->SetScale(config.keys.partyPortrait[1], config.scales.partyScale, config.scales.partyScale);

	uiRegistry->SetEnabled(config.keys.leaderPortrait, true);
	uiRegistry->SetEnabled(config.keys.partyPortrait[0], true);
	uiRegistry->SetEnabled(config.keys.partyPortrait[1], true);

	uiRegistry->Ensure(config.keys.tacticLevelup);
	uiRegistry->SetEnabled(config.keys.tacticLevelup, true);
	uiAnimSys->SetOpacity(config.keys.tacticLevelup, 0.f);
	uiRegistry->SetLocalPos(config.keys.tacticLevelup, 0.f, 0.f);

	uiRegistry->Ensure(config.keys.tacticBarBack);
	uiRegistry->SetEnabled(config.keys.tacticBarBack, true);
	uiAnimSys->SetOpacity(config.keys.tacticBarBack, 0.f);

	uiRegistry->Ensure(L"pinch_1");
	uiRegistry->Ensure(L"pinch_2");
	uiRegistry->SetEnabled(L"pinch_1", true);
	uiRegistry->SetEnabled(L"pinch_2", true);
	uiAnimSys->SetOpacity(L"pinch_1", 0.f);
	uiAnimSys->SetOpacity(L"pinch_2", 0.f);

	uiRegistry->Ensure(config.keys.damageGlowLeft);
	uiRegistry->Ensure(config.keys.damageGlowRight);
	uiRegistry->SetEnabled(config.keys.damageGlowLeft, true);
	uiRegistry->SetEnabled(config.keys.damageGlowRight, true);
	uiAnimSys->SetOpacity(config.keys.damageGlowLeft, 0.f);
	uiAnimSys->SetOpacity(config.keys.damageGlowRight, 0.f);

	rt.hpLeaderDigits.anchorX = config.layout.leaderPos.x + 360.f;
	rt.hpLeaderDigits.anchorY = config.layout.leaderPos.y + 95.f;
	rt.hpLeaderDigits.gapX = 19.f;
	rt.hpLeaderDigits.scaleX = 0.8f;
	rt.hpLeaderDigits.scaleY = 0.8f;

	rt.hpPartyDigits[0].anchorX = config.layout.partyPos[0].x + 140.f;
	rt.hpPartyDigits[0].anchorY = config.layout.partyPos[0].y + 90.f;
	rt.hpPartyDigits[0].gapX = 19.f;
	rt.hpPartyDigits[0].scaleX = 0.8f;
	rt.hpPartyDigits[0].scaleY = 0.8f;

	rt.hpPartyDigits[1].anchorX = config.layout.partyPos[1].x + 150.f;
	rt.hpPartyDigits[1].anchorY = config.layout.partyPos[1].y + 80.f;
	rt.hpPartyDigits[1].gapX = 19.f;
	rt.hpPartyDigits[1].scaleX = 0.8f;
	rt.hpPartyDigits[1].scaleY = 0.8f;

	rt.apLeaderDigits.anchorX = config.layout.leaderPos.x + 360.f;
	rt.apLeaderDigits.anchorY = config.layout.leaderPos.y + 5.f;
	rt.apLeaderDigits.gapX = 25.f;
	rt.apLeaderDigits.scaleX = 1.2f;
	rt.apLeaderDigits.scaleY = 1.2f;

	rt.tacticLvDigits.anchorX = config.layout.leaderPos.x + 35.f;
	rt.tacticLvDigits.anchorY = config.layout.leaderPos.y + 147.f;
	rt.tacticLvDigits.gapX = 19.f;
	rt.tacticLvDigits.scaleX = 1.f;
	rt.tacticLvDigits.scaleY = 1.f;
}

void BattleHUDPresenter::EnsureBars()
{
	uiRegistry->Ensure(config.bars.leader.hpFront);
	uiRegistry->Ensure(config.bars.leader.stunFront);
	uiRegistry->SetEnabled(config.bars.leader.hpFront, true);
	uiRegistry->SetEnabled(config.bars.leader.stunFront, true);
	uiAnimSys->SetOpacity(config.bars.leader.hpFront, 1.f);
	uiAnimSys->SetOpacity(config.bars.leader.stunFront, 1.f);

	for (int i = 0; i < 2; ++i)
	{
		const auto& k = config.bars.party[(size_t)i];
		uiRegistry->Ensure(k.hpFront);
		uiRegistry->Ensure(k.stunFront);
		uiRegistry->SetEnabled(k.hpFront, true);
		uiRegistry->SetEnabled(k.stunFront, true);
		uiAnimSys->SetOpacity(k.hpFront, 1.f);
		uiAnimSys->SetOpacity(k.stunFront, 1.f);
	}
}

void BattleHUDPresenter::WireSubs()
{
	if (wired) return;

	listenerIds.push_back(
		eventBus->Subscribe(BattleBusEventType::LeaderChanged, [&](const BattleEvent&)
			{
				RefreshLeaderAndParty();
				ApplyLeaderPortrait();
				ApplyPartyPortraits();
				SyncAllImmediate();
			})
	);

	listenerIds.push_back(eventBus->Subscribe(BattleBusEventType::TacticChanged,
		[&](const BattleEvent& e) { if (auto p = get_if<EventPayload_Tactic>(&e.payload)) OnTacticEvent(*p); }));

	listenerIds.push_back(eventBus->Subscribe(BattleBusEventType::TacticLevelUp,
		[&](const BattleEvent& e) { if (auto p = get_if<EventPayload_Tactic>(&e.payload)) OnTacticEvent(*p); }));

	listenerIds.push_back(eventBus->Subscribe(BattleBusEventType::TacticMaxBlinkOn,
		[&](const BattleEvent& e) { if (auto p = get_if<EventPayload_Tactic>(&e.payload)) OnTacticEvent(*p); }));

	listenerIds.push_back(eventBus->Subscribe(BattleBusEventType::TacticMaxBlinkOff,
		[&](const BattleEvent& e) { if (auto p = get_if<EventPayload_Tactic>(&e.payload)) OnTacticEvent(*p); }));

	listenerIds.push_back(eventBus->Subscribe(BattleBusEventType::ResolveDamageApplied,
		[&](const BattleEvent& e) { if (auto p = get_if<EventPayload_Damage>(&e.payload)) OnDamageShake(e.subjectEntity, p->targetEntity, p->damageAmount, p->isCritical); }));

	wired = true;
}

void BattleHUDPresenter::UnWireSubs()
{
	if (!wired) return;
	for (auto id : listenerIds) eventBus->Unsubscribe(id);
	listenerIds.clear();
	wired = false;
}

void BattleHUDPresenter::RefreshLeaderAndParty()
{
	rt.leader = timelineSys->GetLeader();

	rt.party = { invalidEntity, invalidEntity };
	const BattleParty& allies = sessionSys->GetAllies();
	int w = 0;
	for (int i = 0; i < allies.memberCount && w < 2; ++i)
	{
		const EntityID e = allies.members[i];
		if (e == 0u) continue;
		if (e == rt.leader)     continue;
		rt.party[w++] = e;
	}
}

void BattleHUDPresenter::ApplyLeaderPortrait()
{
	const wstring& newKey = dataSys->GetTextureKey(rt.leader, UITextureSlot::BattleCharacterIcon);
	if (newKey == rt.appliedLeaderTex) return;
	uiRegistry->SetWidgetTexture(config.keys.leaderPortrait, newKey);
	rt.appliedLeaderTex = newKey;
}

void BattleHUDPresenter::ApplyPartyPortraits()
{
	for (size_t i = 0; i < 2; ++i)
	{
		const EntityID e = rt.party[i];
		const wstring& newKey = dataSys->GetTextureKey(e, UITextureSlot::BattleCharacterIcon);
		if (newKey == rt.appliedPartyTex[i]) continue;
		uiRegistry->SetWidgetTexture(config.keys.partyPortrait[i], newKey);
		rt.appliedPartyTex[i] = newKey;
	}
}

void BattleHUDPresenter::SyncAllImmediate()
{
	rt.leaderHP.ratio = attributeSys->GetHpRatio01(rt.leader);
	rt.leaderStun.ratio = 0.f;

	UIFillSpec hpSpec = { UIFillAxis::X, UIFillOrigin::Start, 3.0f, 6.0f };
	UIFillSpec stunSpec = { UIFillAxis::X, UIFillOrigin::Start, 10.0f, 8.0f };

	uiAnimSys->FillSetImmediate(config.bars.leader.hpFront, rt.leaderHP.ratio, hpSpec);
	uiAnimSys->FillSetImmediate(config.bars.leader.stunFront, rt.leaderStun.ratio, stunSpec);

	for (int i = 0; i < 2; ++i)
	{
		const EntityID e = rt.party[i];
		rt.partyHP[i].ratio = attributeSys->GetHpRatio01(e);
		rt.partyStun[i].ratio = 0.f;

		const auto& k = config.bars.party[(size_t)i];
		uiAnimSys->FillSetImmediate(k.hpFront, rt.partyHP[i].ratio, hpSpec);
		uiAnimSys->FillSetImmediate(k.stunFront, rt.partyStun[i].ratio, stunSpec);
		rt.prevHpRatioForGlow[i] = rt.partyHP[i].ratio;
	}
}

void BattleHUDPresenter::UpdateHPFront()
{
	UIFillSpec hpSpec = { UIFillAxis::X, UIFillOrigin::Start, 3.0f, 6.0f };

	const float lr = attributeSys->GetHpRatio01(rt.leader);
	if (lr != rt.leaderHP.ratio)
	{
		rt.leaderHP.ratio = lr;
		uiAnimSys->FillTo(config.bars.leader.hpFront, lr, hpSpec);
	}

	for (int i = 0; i < 2; ++i)
	{
		const EntityID e = rt.party[i];
		const float r = attributeSys->GetHpRatio01(e);
		if (r != rt.partyHP[i].ratio)
		{
			rt.partyHP[i].ratio = r;
			uiAnimSys->FillTo(config.bars.party[(size_t)i].hpFront, r, hpSpec);
		}
	}
}

void BattleHUDPresenter::UpdateStunFront()
{
	UIFillSpec stunSpec = { UIFillAxis::X, UIFillOrigin::Start, 10.0f, 8.0f };

	const float lr = attributeSys->GetStunRatio01(rt.leader);
	if (lr != rt.leaderStun.ratio)
	{
		rt.leaderStun.ratio = lr;
		uiAnimSys->FillTo(config.bars.leader.stunFront, lr, stunSpec);
	}

	for (int i = 0; i < 2; ++i)
	{
		const EntityID e = rt.party[i];
		const float r = attributeSys->GetStunRatio01(e);
		if (r != rt.partyStun[i].ratio)
		{
			rt.partyStun[i].ratio = r;
			uiAnimSys->FillTo(config.bars.party[(size_t)i].stunFront, r, stunSpec);
		}
	}
}

void BattleHUDPresenter::UpdateTexts()
{
	if (rt.leader == 0u) return;

	const auto hpLeader = attributeSys->GetHp(rt.leader);
	SetNumberSlot(rt.hpLeaderDigits, hpLeader.cur);

	for (int i = 0; i < 2; ++i)
	{
		const EntityID e = rt.party[i];
		if (e == 0u)
		{
			SetNumberSlot(rt.hpPartyDigits[(size_t)i], -1);
			continue;
		}

		const auto hp = attributeSys->GetHp(e);
		SetNumberSlot(rt.hpPartyDigits[(size_t)i], hp.cur);
	}

	int curAp = 0;
	int maxAp = 0;
	timelineSys->GetApSnapshot(rt.leader, curAp, maxAp);
	SetNumberSlot(rt.apLeaderDigits, curAp);

	const auto& tac = tacticSys->GetState();
	SetNumberSlot(rt.tacticLvDigits, tac.level);
}

void BattleHUDPresenter::ApplyTacticFromState(int level, int pips, bool maxBlink)
{
	level = max(1, min(level, 5));
	pips = max(0, min(pips, 5));

	rt.tactic.level = level;
	rt.tactic.pips = pips;

	const int lvIdx = level - 1;

	for (int i = 0; i < 5; ++i)
	{
		const auto& kF = config.tactic.front[(size_t)i];
		const auto& kU = config.tactic.full[(size_t)i];

		uiAnimSys->SetOpacity(kU, 0.f);

		if (i < lvIdx)
		{
			uiAnimSys->SetOpacity(kF, 1.f);
			uiRegistry->SetFillRatioX(kF, 1.f);
		}
		else if (i == lvIdx)
		{
			const float r = TacticFill(pips);
			uiAnimSys->SetOpacity(kF, 1.f);
			uiRegistry->SetFillRatioX(kF, r);
		}
		else
		{
			uiAnimSys->SetOpacity(kF, 0.f);
			uiRegistry->SetFillRatioX(kF, 0.f);
		}
	}

	rt.tactic.blink.active = maxBlink;
}

void BattleHUDPresenter::OnTacticEvent(const EventPayload_Tactic& p)
{
	switch (p.type)
	{
	case TacticEventType::Changed:
		ApplyTacticFromState(p.level, p.pips, p.maxBlink);
		break;
	case TacticEventType::LevelUp:
	{
		ApplyTacticFromState(p.level, p.pips, false);
		rt.tactic.revealIdx = p.level - 1;
		rt.tactic.revealA = 0.f;
		uiAnimSys->SetOpacity(config.tactic.front[(size_t)rt.tactic.revealIdx], 0.f);
		ShowLevelUpBanner();
		break;
	}
	case TacticEventType::MaxBlinkOn:
		rt.tactic.blink.active = true;
		rt.tactic.blink.t = 0.f;
		break;
	case TacticEventType::MaxBlinkOff:
		rt.tactic.blink.active = false;
		for (int i = 0; i < 5; ++i) 
			uiAnimSys->SetOpacity(config.tactic.full[(size_t)i], 0.f);
		break;
	}
}

void BattleHUDPresenter::EnsureTacticBars()
{
	for (int i = 0; i < 5; ++i)
	{
		uiRegistry->Ensure(config.tactic.front[(size_t)i]);
		uiRegistry->Ensure(config.tactic.full[(size_t)i]);

		uiRegistry->SetEnabled(config.tactic.front[(size_t)i], true);
		uiRegistry->SetEnabled(config.tactic.full[(size_t)i], true);

		uiAnimSys->SetOpacity(config.tactic.full[(size_t)i], 0.f);
	}
}

void BattleHUDPresenter::TickTacticReveal(float dt)
{
	if (rt.tactic.revealIdx < 0) return;
	const float speed = 4.f;
	rt.tactic.revealA = min(1.f, rt.tactic.revealA + speed * dt);
	const auto& k = config.tactic.front[(size_t)rt.tactic.revealIdx];
	uiAnimSys->SetOpacity(k, rt.tactic.revealA);
	if (rt.tactic.revealA >= 1.f) rt.tactic.revealIdx = -1;
}

void BattleHUDPresenter::TickTacticBlink(float dt)
{
	auto& b = rt.tactic.blink;
	if (!b.active) return;
	b.t += dt;
	const float a = BlinkAlpha(b);
	for (int i = 0; i < 5; ++i)
		uiAnimSys->SetOpacity(config.tactic.full[(size_t)i], a);
}

void BattleHUDPresenter::TickPinch(float dt)
{
	const float thOn = 0.50f;
	const float thOff = 0.55f;

	const float r0 = rt.partyHP[0].ratio;
	const float r1 = rt.partyHP[1].ratio;

	const bool want0 = rt.pinch[0].active ? (r0 < thOff) : (r0 < thOn);
	const bool want1 = rt.pinch[1].active ? (r1 < thOff) : (r1 < thOn);

	if (want0)
	{
		if (!rt.pinch[0].active)
		{
			rt.pinch[0].active = true;
			rt.pinch[0].t = 0.f;
			uiAnimSys->SetOpacity(L"pinch_1", 1.f);
		}
		rt.pinch[0].t += dt;
		const float s0 = rt.pinch[0].minScale + (rt.pinch[0].maxScale - rt.pinch[0].minScale) * (0.5f + 0.5f * sinf(XM_2PI * rt.pinch[0].freq * rt.pinch[0].t));
		uiAnimSys->SetScale(L"pinch_1", s0, s0);
	}
	else
	{
		if (rt.pinch[0].active)
		{
			rt.pinch[0].active = false;
			uiAnimSys->SetOpacity(L"pinch_1", 0.f);
			uiAnimSys->SetScale(L"pinch_1", 1.f, 1.f);
		}
	}

	if (want1)
	{
		if (!rt.pinch[1].active)
		{
			rt.pinch[1].active = true;
			rt.pinch[1].t = 0.f;
			uiAnimSys->SetOpacity(L"pinch_2", 1.f);
		}
		rt.pinch[1].t += dt;
		const float s1 = rt.pinch[1].minScale + (rt.pinch[1].maxScale - rt.pinch[1].minScale) * (0.5f + 0.5f * sinf(XM_2PI * rt.pinch[1].freq * rt.pinch[1].t));
		uiAnimSys->SetScale(L"pinch_2", s1, s1);
	}
	else
	{
		if (rt.pinch[1].active)
		{
			rt.pinch[1].active = false;
			uiAnimSys->SetOpacity(L"pinch_2", 0.f);
			uiAnimSys->SetScale(L"pinch_2", 1.f, 1.f);
		}
	}
}

void BattleHUDPresenter::TickDamageGlow(float dt)
{
	auto tickOne = [&](DamageGlowState& s, const wstring& key)
		{
			if (!s.active) return;

			s.t += dt;
			if (s.t >= s.duration)
			{
				s.active = false;
				s.t = 0.f;
				uiAnimSys->SetOpacity(key, 0.f);
				return;
			}

			const float phase = XM_2PI * s.freq * s.t;
			const float wave = 0.5f + 0.5f * sinf(phase); 
			const float a = s.minA + (s.maxA - s.minA) * wave;

			uiAnimSys->SetOpacity(key, a);
		};

	tickOne(rt.glowLeft, config.keys.damageGlowLeft);
	tickOne(rt.glowRight, config.keys.damageGlowRight);
}

void BattleHUDPresenter::ShowLevelUpBanner()
{
	const float baseX  = 0.f;
	const float baseY  = 0.f;
	const float inDx   = 100.f;
	const float inDur  = 1.2f;
	const float hold   = 0.3f;
	const float outDur = 0.5f;

	const auto& banner = config.keys.tacticLevelup;
	const auto& back   = config.keys.tacticBarBack;

	uiRegistry->SetLocalPos(banner, baseX - inDx, baseY);
	uiAnimSys->SetOpacity(banner, 0.5f);
	uiAnimSys->PlaySlideOnce(banner, baseX - inDx, baseY, baseX, baseY, inDur);
	uiAnimSys->PlayFadeOnce(banner, 0.5f, 1.0f, inDur);

	uiAnimSys->SetOpacity(back, 0.5f);
	uiAnimSys->SetScale(back, 1.f, 0.8f);

	uiAnimSys->PlayScaleOnce(back, 1.f, 0.8f, 1.f, 1.f, inDur, UIEasing::EaseWave);
	uiAnimSys->PlayFadeOnce(back, 0.5f, 1.0f, inDur);

	rt.tactic.lvBannerVisible = true;
	rt.tactic.lvBannerFadingOut = false;
	rt.tactic.lvBannerTimer = hold + outDur;
}

void BattleHUDPresenter::TickLevelUpBanner(float dt)
{
	if (!rt.tactic.lvBannerVisible) return;

	const auto& banner = config.keys.tacticLevelup;
	const auto& back   = config.keys.tacticBarBack;

	if (!rt.tactic.lvBannerFadingOut)
	{
		rt.tactic.lvBannerTimer -= dt;
		if (rt.tactic.lvBannerTimer <= 0.f)
		{
			rt.tactic.lvBannerFadingOut = true;

			const float outDur = 0.30f; 
			uiAnimSys->PlayFadeOnce(banner, 1.0f, 0.0f, outDur);
			uiAnimSys->PlayScaleOnce(back, 1.f, 1.f, 1.f, 0.0f, outDur);
			uiAnimSys->PlayFadeOnce(back, 1.0f, 0.0f, outDur);
		}
	}
	else
	{
		rt.tactic.lvBannerTimer -= dt;
		if (rt.tactic.lvBannerTimer <= -0.05f)
		{
			rt.tactic.lvBannerVisible = false;
			rt.tactic.lvBannerFadingOut = false;
			uiAnimSys->SetOpacity(back, 0.0f);
			uiAnimSys->SetScale(back, 1.f, 0.05f);
			uiAnimSys->SetOpacity(banner, 0.0f);
		}
	}
}

void BattleHUDPresenter::OnDamageShake(EntityID attacker, EntityID target, int dmg, bool critical)
{
	const int maxHp = attributeSys->GetHp(target).max;
	float ratio = (maxHp > 0) ? (float)dmg / (float)maxHp : 1.f;
	ratio = min(ratio, 1.f);

	float scale = 0.9f + 1.3f * sqrtf(ratio);
	if (critical) scale *= 1.18f;
	scale = clamp(scale, 0.9f, 2.0f);

	UIShakeSpec spec;
	spec.ampX = 8.f;
	spec.ampY = 3.5f;
	spec.freq = 3.f;
	spec.decay = 1.3f;
	spec.dur = 0.55f;
	spec.phaseX = 0.f;
	spec.phaseY = 1.57f;
	float dirX = (rand() % 2 == 0) ? 1.f : -1.f;
	spec.ampX += (((rand() % 100) / 100.f) - 0.5f) * 0.8f;
	spec.ampY += (((rand() % 100) / 100.f) - 0.5f) * 0.8f;

	auto kick = [&](const wstring& key) { uiAnimSys->PlayShakeOnce(key, spec, dirX, scale); };

	if (target == rt.leader)
	{
		kick(config.keys.leaderPortrait);
	}
	else
	{
		for (int i = 0; i < 2; ++i)
		{
			if (rt.party[i] == target)
				kick(config.keys.partyPortrait[(size_t)i]);
		}
	}

	constexpr float threshold = 0.99f; //

	for (int i = 0; i < 2; ++i)
	{
		const EntityID e = rt.party[i];
		if (e == 0u) continue; 

		const auto hp = attributeSys->GetHp(e);
		if (hp.max <= 0) continue;

		const float hpRatio = (float)hp.cur / (float)hp.max;

		const float prev = rt.prevHpRatioForGlow[i];
		rt.prevHpRatioForGlow[i] = hpRatio;

		if (prev > threshold && hpRatio <= threshold)
		{
			const bool rightSide = IsEntityOnRightSide(e);

			if (rightSide)
			{
				rt.glowRight.active = true;
				rt.glowRight.t = 0.f;
				rt.glowRight.duration = 5.f;
			}
			else
			{
				rt.glowLeft.active = true;
				rt.glowLeft.t = 0.f;
				rt.glowLeft.duration = 5.f;
			}
		}
	}
}

bool BattleHUDPresenter::IsEntityOnRightSide(EntityID entity) const
{
	float x = 0.f;

	if (entity == rt.leader)
		x = rt.hpLeaderDigits.anchorX;
	else if (entity == rt.party[0])
		x = rt.hpPartyDigits[0].anchorX;
	else if (entity == rt.party[1])
		x = rt.hpPartyDigits[1].anchorX;

	return x > 0.f;
}

void BattleHUDPresenter::SetNumberSlot(DigitSlot& slot, int value)
{
	if (value < 0)
	{
		for (auto& k : slot.keys)
			uiRegistry->SetEnabled(k, false);
		return;
	}

	assert(value >= 0);
	wstring s = to_wstring(value);
	const int need = (int)s.size();
	const int have = (int)slot.keys.size();

	if (need > have)
	{
		for (int i = have; i < need; ++i)
		{
			wstring instKey = L"battle_num#" + to_wstring((uintptr_t)&slot) + L"_" + to_wstring(i);
			uiRegistry->EnsureClone(L"battleletter_0", instKey);
			uiRegistry->SetEnabled(instKey, true);
			uiAnimSys->SetOpacity(instKey, 1.f);
			uiAnimSys->SetScale(instKey, slot.scaleX, slot.scaleY);   // ← 슬롯 스케일 적용
			slot.keys.push_back(instKey);
		}
	}
	else if (need < have)
	{
		for (int i = have - 1; i >= need; --i)
		{
			uiRegistry->SetEnabled(slot.keys[(size_t)i], false);
			slot.keys.pop_back();
		}
	}

	for (int i = 0; i < need; ++i)
	{
		int d = int(s[(size_t)i] - L'0');
		assert(d >= 0 && d <= 9);
		wstring texKey = L"battleletter_" + to_wstring(d);
		uiRegistry->SetWidgetTexture(slot.keys[(size_t)i], texKey);
		uiRegistry->SetEnabled(slot.keys[(size_t)i], true);
		uiAnimSys->SetScale(slot.keys[(size_t)i], slot.scaleX, slot.scaleY); // 재설정 한번 더
	}

	const int   count = need;
	const float W = (count > 0) ? (count - 1) * slot.gapX : 0.f;
	const float startX = slot.anchorX - W;
	const float baseY = slot.anchorY;

	for (int i = 0; i < count; ++i)
	{
		const float x = startX + slot.gapX * float(i);
		const float y = baseY;
		uiRegistry->SetLocalPos(slot.keys[(size_t)i], x, y);
	}
}