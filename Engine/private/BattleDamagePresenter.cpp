#include "Enginepch.h"
#include "BattleDamagePresenter.h"
#include "SoundSystem.h"

inline static int DigitCount(int v) { return (v == 0) ? 1 : (int)floor(log10((double)abs(v))) + 1; }
static float MeasureDigitAdvance(UIRegistry* uiReg, const DamageFontSet& font)
{
	float mx = 0.f;
	for (int d = 0; d <= 9; ++d)
	{
		auto [w, h] = uiReg->GetOrCacheTexSize(font.digits[d]);
		mx = max(mx, w);
	}
	return mx;
}
// ========================================================================================================
void BattleDamagePresenter::OnBoot()
{
	uiRegistry  = &registry.Get<UIRegistry>();
	uiAnimSys   = &registry.Get<UIAnimSystem>();
	eventBus    = &registry.Get<BattleEventBus>();
	timelineSys = &registry.Get<BattleTimelineSystem>();
	sessionSys  = &registry.Get<BattleSessionSystem>();
	soundSys    = &registry.Get<SoundSystem>();
	actives.reserve(64);

	DamageFontSet font{};
	for (int i = 0; i <= 9; ++i)
		font.digits[i] = L"dmgletter_" + to_wstring(i);

	DamageHudSpec spec{};
	spec.baseX      = 1100.f; 
	spec.baseY      = 0.f;
	spec.gapX       = 30.f;
	spec.inSlideDx  = 30.f;
	spec.inDur      = 0.4f;
	spec.holdDur    = 2.f;
	spec.outSlideDx = 30.f;
	spec.outDur     = 0.4f;

	SetFont(font);
	SetSpec(spec);

	/// Chain ========
	for (int i = 0; i <= 9; ++i)
		chainFont.digits[i] = L"chainletter_" + to_wstring(i);

	chain.anchorX = spec.baseX - 25.f;
	chain.anchorY = spec.baseY - 125.f; // 원하는 만큼 위로 올리면 됨
}

void BattleDamagePresenter::Tick(float dt)
{
	if (accum.active)
	{
		accum.timer += dt;
		if (accum.timer >= accumWindow)
		{
			const int total = max(accum.sum, 0);
			if (total > 0)
			{
				if (!playing) 
					SpawnDamage(total);
				else
				{
					if (!actives.empty())
					{
						DamageNumber& cur = actives.front();
						cur.targetValue += total;  
						cur.elapsed = 0.f;         
						cur.lifeTime = spec.inDur + spec.holdDur + spec.outDur; 
					}
					else
						SpawnDamage(total);
				}
			}
			accum = Burst{};
		}
	}

	for (size_t i = 0; i < actives.size(); )
	{
		DamageNumber& n = actives[i];
		if (!n.active) { actives.erase(actives.begin() + i); continue; }

		if (n.curValue < n.targetValue)
		{
			n.countAcc += countPerSec * dt;
			while (n.countAcc >= 1.f && n.curValue < n.targetValue)
			{
				n.curValue += 1;
				n.countAcc -= 1.f;
			}
			SetDigitsTextures(n, n.curValue);
		}

		n.elapsed += dt;

		const float inHold = spec.inDur + spec.holdDur;

		if (n.elapsed >= inHold && n.elapsed - dt < inHold)
		{
			AnimateOut(n.keys);      
			AnimateOutTotal();      
		}

		if (n.elapsed >= n.lifeTime)
		{
			for (const auto& k : n.keys)
				uiRegistry->SetEnabled(k, false);
			uiRegistry->SetEnabled(L"total_damage", false);
			n.Reset();
			actives.erase(actives.begin() + i);
			continue;
		}
		++i;
	}

	if (playing && actives.empty())
	{
		playing = false; 
		if (!queue.empty())
		{
			const int next = queue.front(); queue.pop_front();
			SpawnDamage(next);
		}
	}
	if (total.active && total.curValue < total.targetValue)
	{
		total.countAcc += countPerSec * dt;

		while (total.countAcc >= 1.f && total.curValue < total.targetValue)
		{
			total.curValue += 1;
			total.countAcc -= 1.f;
		}

		SetDigitsTextures(total, total.curValue);

		const wstring totalKey = L"total_damage";
		uiRegistry->SetEnabled(totalKey, true);
		uiAnimSys->SetOpacity(totalKey, 1.f);
	}
}

void BattleDamagePresenter::SpawnDamage(int dmg)
{  
	const _uint sid = ++spawnSerial;

	DamageNumber num{};
	num.spawnId = sid;
	num.curValue = 0;
	num.targetValue = max(0, dmg);
	num.countAcc = 0.f;

	num.anchorX = spec.baseX; 
	num.anchorY = spec.baseY; 
	num.y = num.anchorY;

	const float adv = MeasureDigitAdvance(uiRegistry, font);
	num.advanceX = max(spec.gapX, adv);

	const int digits = DigitCount(num.targetValue);
	for (int i = 0; i < digits; ++i)
	{
		const wstring arche = font.digits[0];
		const wstring inst = arche + L"#" + to_wstring(num.spawnId) + L"_" + to_wstring(i);
		EnsureDigit(arche, inst);
		uiRegistry->SetEnabled(inst, true);
		uiAnimSys->SetOpacity(inst, 0.f);
		num.keys.push_back(inst);
	}

	num.lifeTime = spec.inDur + spec.holdDur + spec.outDur;
	num.elapsed = 0.f;
	num.active = true;

	RelayoutRightAligned(num);
	AnimateIn(num.keys, 0.f, 0.f);

	actives.push_back(num);
	playing = true;
	{
		const wstring totalKey = L"total_damage";
		uiRegistry->SetEnabled(totalKey, true);
		uiAnimSys->SetOpacity(totalKey, 0.f);
		uiAnimSys->PlaySlideOnce(totalKey, spec.inSlideDx, 0.f, 0.f, 0.f, spec.inDur, spec.inEase);
		uiAnimSys->PlayFadeOnce(totalKey, 0.f, 1.f, spec.inDur, spec.inEase);
	}
}

void BattleDamagePresenter::Enter()
{
	chain.advanceX = MeasureDigitAdvance(uiRegistry, chainFont);
	const float adv = MeasureDigitAdvance(uiRegistry, font);

	total.spawnId = ++spawnSerial;
	total.anchorX = spec.baseX;
	total.anchorY = spec.baseY;
	total.y = total.anchorY;
	total.advanceX = max(spec.gapX, adv);
	total.curValue = 0;
	total.targetValue = 0;
	total.countAcc = 0.f;
	total.elapsed = 0.f;
	total.lifeTime = 0.f;
	total.active = true;
	total.keys.clear();
}

void BattleDamagePresenter::Wire()
{
	listenerIds.push_back(eventBus->Subscribe(BattleBusEventType::ResolveDamageApplied,
		[&](const BattleEvent& e)
		{
			if (auto p = get_if<EventPayload_Damage>(&e.payload))
			{
				const EntityID a = e.subjectEntity;
				const bool ally =
					(a == timelineSys->GetLeader()) ||
					(a == sessionSys->GetAllies().members[0]) ||
					(a == sessionSys->GetAllies().members[1]);
				if (!ally) return;

				const int dmg = max(0, p->damageAmount);
				if (dmg <= 0) return;

				total.targetValue += dmg;
			}
		}));
}

void BattleDamagePresenter::UnWire()
{

}

void BattleDamagePresenter::EnsureDigit(const wstring& key, const wstring& instKey)
{
	uiRegistry->EnsureClone(key, instKey);
}

void BattleDamagePresenter::BuildDigits(int value, vector<wstring>& outKeys, _uint spawnId)
{
	assert(value >= 0);
	if (value == 0)
	{
		const wstring instKey = font.digits[0] + L"#" + to_wstring(spawnId) + L"_0";
		EnsureDigit(font.digits[0], instKey);
		outKeys.push_back(instKey);
		return;
	}

	wstring s = to_wstring(value);
	outKeys.reserve(s.size());
	for (size_t i = 0; i < s.size(); ++i)
	{
		int d = int(s[i] - L'0');
		assert(d >= 0 && d <= 9);
		const wstring arche = font.digits[d];
		const wstring inst = arche + L"#" + to_wstring(spawnId) + L"_" + to_wstring(i);
		EnsureDigit(arche, inst);
		outKeys.push_back(inst);
	}
}

void BattleDamagePresenter::AnimateIn(const vector<wstring>& keys, float x, float y)
{
	const size_t count = keys.size();
	for (size_t i = 0; i < count; ++i)
	{
		const wstring& k = keys[i];

		uiAnimSys->SetOpacity(k, 0.f);
		uiAnimSys->SetOffSet(k, 0.f, 0.f);

		uiAnimSys->PlaySlideOnce(k, spec.inSlideDx, 0.f, 0.f, 0.f, spec.inDur, spec.inEase);
		uiAnimSys->PlayFadeOnce(k, 0.f, 1.f, spec.inDur, spec.inEase);
	}
}

void BattleDamagePresenter::AnimateOut(const vector<wstring>& keys)
{
	for (const auto& k : keys)
	{
		uiAnimSys->PlaySlideOnce(k, 0.f, 0.f, spec.outSlideDx, 0.f, spec.outDur, spec.outEase);
		uiAnimSys->PlayFadeOnce(k, 1.f, 0.f, spec.outDur, spec.outEase);
	}
}

void BattleDamagePresenter::RelayoutRightAligned(DamageNumber& n)
{
	const int   count = static_cast<int>(n.keys.size());
	const float W = (count > 0) ? (count - 1) * n.advanceX : 0.f;

	const float startX = n.anchorX - W;
	const float baseY = n.y;

	for (int i = 0; i < count; ++i)
	{
		const float x = startX + n.advanceX * float(i);  
		const float y = baseY + spec.gapY * float(i);   

		uiRegistry->SetLocalPos(n.keys[(size_t)i], x, y); 
	}

	const wstring totalKey = L"total_damage";
	UIInstance& totalInst = uiRegistry->Ensure(totalKey);

	if (count > 0)
	{
		const float lastX = startX + n.advanceX * float(count - 1);
		const float labelX = lastX + n.advanceX + 25.f;  
		const float labelY = baseY;

		uiRegistry->SetLocalPos(totalKey, labelX, labelY);
	}
	else
		uiRegistry->SetLocalPos(totalKey, n.anchorX, baseY);
}

void BattleDamagePresenter::SetDigitsTextures(DamageNumber& n, int value)
{
	wstring s = to_wstring(max(0, value));
	const int need = (int)s.size();
	const int have = (int)n.keys.size();

	if (need > have)
	{
		for (int i = have; i < need; ++i)
		{
			const int d = int(s[i] - L'0');
			const wstring arche = font.digits[d];
			const wstring inst = arche + L"#" + to_wstring(n.spawnId) + L"_" + to_wstring(i);
			EnsureDigit(arche, inst);
			uiRegistry->SetEnabled(inst, true);
			uiAnimSys->SetOpacity(inst, 1.f);
			n.keys.push_back(inst);
		}
	}
	else if (need < have)
	{
		for (int i = have - 1; i >= need; --i)
		{
			uiRegistry->SetEnabled(n.keys[(size_t)i], false);
			n.keys.pop_back();
		}
	}

	for (int i = 0; i < need; ++i)
	{
		const int d = int(s[i] - L'0');
		uiRegistry->SetWidgetTexture(n.keys[(size_t)i], font.digits[d]);
	}

	RelayoutRightAligned(n);
}

void BattleDamagePresenter::AnimateOutTotal()
{
	const wstring totalKey = L"total_damage";
	uiAnimSys->PlaySlideOnce(totalKey, 0.f, 0.f, spec.outSlideDx, 0.f, spec.outDur, spec.outEase);
	uiAnimSys->PlayFadeOnce(totalKey, 1.f, 0.f, spec.outDur, spec.outEase);
}

void BattleDamagePresenter::SetChain(int chainValue)
{
	const bool wasVisible = chain.visible;
	const int  prevValue = chain.value;

	const wstring labelKeys[] = { L"chain", L"chain_x", L"chain_damage" };
	const wstring mulKey = L"battle_chain_mul";

	if (chainValue <= 0)
	{
		if (wasVisible)
		{
			for (const auto& k : chain.keys)
			{
				uiAnimSys->PlaySlideOnce( k, 0.f, 0.f, spec.outSlideDx, 0.f, spec.outDur, spec.outEase);
				uiAnimSys->PlayFadeOnce( k, 1.f, 0.f, spec.outDur, spec.outEase);
			}
			for (const auto& key : labelKeys)
			{
				UIInstance& inst = uiRegistry->Ensure(key);
				uiRegistry->SetEnabled(key, true);

				uiAnimSys->PlaySlideOnce( key, 0.f, 0.f, spec.outSlideDx, 0.f, spec.outDur, spec.outEase); 
				uiAnimSys->PlayFadeOnce( key, 1.f, 0.f, spec.outDur, spec.outEase);
			}
			{
				UIInstance& inst = uiRegistry->Ensure(mulKey);
				uiRegistry->SetEnabled(mulKey, true);
				uiAnimSys->PlaySlideOnce( mulKey, 0.f, 0.f, spec.outSlideDx, 0.f, spec.outDur, spec.outEase);
				uiAnimSys->PlayFadeOnce( mulKey, 1.f, 0.f, spec.outDur, spec.outEase);
			}
		}

		chain.value = 0;
		chain.visible = false;
		return;
	}
	chain.value = chainValue;
	chain.visible = true;

	wstring s = to_wstring(chainValue);
	const int need = (int)s.size();
	const int have = (int)chain.keys.size();

	if (need > have)
	{
		for (int i = have; i < need; ++i)
		{
			const wstring arche = chainFont.digits[0];
			const wstring inst = arche + L"#chain_" + to_wstring(i);

			uiRegistry->EnsureClone(arche, inst);
			uiRegistry->SetEnabled(inst, true);
			uiAnimSys->SetOpacity(inst, 1.f);
			uiAnimSys->SetOffSet(inst, 0.f, 0.f);

			chain.keys.push_back(inst);
		}
	}
	else if (need < have)
	{
		for (int i = have - 1; i >= need; --i)
		{
			uiRegistry->SetEnabled(chain.keys[(size_t)i], false);
			chain.keys.pop_back();
		}
	}
	for (int i = 0; i < need; ++i)
	{
		int d = int(s[(size_t)i] - L'0');
		const wstring& instKey = chain.keys[(size_t)i];

		uiRegistry->SetWidgetTexture(instKey, chainFont.digits[d]);
		uiRegistry->SetEnabled(instKey, true);

		uiAnimSys->SetOpacity(instKey, 1.f);
		uiAnimSys->SetOffSet(instKey, 0.f, 0.f);
	}

	const int   count = need;
	const float W = (count > 0) ? (count - 1) * chain.advanceX : 0.f;

	const float startX = chain.anchorX - W;
	const float baseY = chain.anchorY;

	for (int i = 0; i < count; ++i)
	{
		const float x = startX + chain.advanceX * float(i);
		const float y = baseY;

		uiRegistry->SetLocalPos(chain.keys[(size_t)i], x, y);
	}

	for (const auto& key : labelKeys)
	{
		UIInstance& inst = uiRegistry->Ensure(key);
		uiRegistry->SetEnabled(key, true);
		uiAnimSys->SetOpacity(key, 1.f);
		uiAnimSys->SetOffSet(key, 0.f, 0.f);
	}

	{
		int   step = max(chainValue - 1, 0);
		float mul = 1.f + 0.05f * float(step);

		wostringstream oss;
		oss.setf(ios::fixed, ios::floatfield);
		oss.precision(2);
		oss << mul;
		wstring mulText = oss.str();

		UIInstance& inst = uiRegistry->Ensure(mulKey);
		uiRegistry->SetEnabled(mulKey, true);
		uiAnimSys->SetOpacity(mulKey, 1.f);
		uiAnimSys->SetOffSet(mulKey, 0.f, 0.f);

		uiRegistry->SetText(mulKey, mulText); 
	}
	if (!wasVisible || prevValue < chainValue)
	{
		soundSys->Play(L"05_skill_chain", 0.1f);
		for (const auto& k : chain.keys)
		{
			uiAnimSys->PlayScaleOnce( k, 1.f, 1.f, 2.f, 2.f, 0.06f, spec.inEase);
			uiAnimSys->PlayScaleOnce( k, 2.f, 2.f, 1.f, 1.f, 0.08f, spec.outEase);
		}
		const wstring scaleKeys[] = { L"chain", L"chain_x", L"chain_damage", mulKey };

		for (const auto& key : scaleKeys)
		{
			UIInstance& inst = uiRegistry->Ensure(key);
			uiRegistry->SetEnabled(key, true);
			uiAnimSys->PlayScaleOnce( key, 1.f, 1.f, 2.f, 2.f, 0.06f, spec.inEase);
			uiAnimSys->PlayScaleOnce( key, 2.f, 2.f, 1.f, 1.f, 0.08f, spec.outEase);
		}
	}
}