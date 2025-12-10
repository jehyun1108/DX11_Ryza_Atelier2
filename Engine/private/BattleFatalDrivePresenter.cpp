#include "Enginepch.h"
#include "BattleFatalDrivePresenter.h"

void BattleFatalDrivePresenter::OnBoot()
{
	uiRegistry  = &registry.Get<UIRegistry>();
	uiAnimSys   = &registry.Get<UIAnimSystem>();
	eventBus    = &registry.Get<BattleEventBus>();
	timelineSys = &registry.Get<BattleTimelineSystem>();
	dataSys     = &registry.Get<CharacterDataSystem>();
	
	letterSpec.keys = {
		L"fataldrive_1", L"fataldrive_2", L"fataldrive_3", L"fataldrive_4", L"fataldrive_5",
		L"fataldrive_6", L"fataldrive_7", L"fataldrive_8", L"fataldrive_9", L"fataldrive_10"
	};
	letterSpec.revealDur   = 0.2f;
	letterSpec.staggerFrac = 0.35f;   
	letterSpec.driftX      = -40.f;
	letterSpec.driftDur    = 2.2f;
	letterSpec.inEase      = UIEasing::EaseOutCubic;
	letterSpec.outEase     = UIEasing::EaseOutCubic;
	letterSpec.outDur      = 0.80f;
	letterSpec.outScale    = 1.30f;
}

void BattleFatalDrivePresenter::Enter()
{
	EnsureInstances();

	rt = FatalDriveRuntime{};
	rt.letters.Reset(letterSpec.keys.size(), letterSpec.revealDur, letterSpec.staggerFrac);

	uiRegistry->SetEnabled(config.keys.ringBack, false);
	uiRegistry->SetEnabled(config.keys.ringFront, false);
	uiRegistry->SetFillRatioX(config.keys.ringFront, 0.f);
	uiAnimSys->SetOpacity(config.keys.portrait, 0.f);

	uiRegistry->SetEnabled(config.keys.bgLeft, true);
	uiRegistry->SetEnabled(config.keys.bgRight, true);
	uiAnimSys->SetOpacity(config.keys.bgLeft, 0.f);
	uiAnimSys->SetOpacity(config.keys.bgRight, 0.f);

	WireSubs();
}

void BattleFatalDrivePresenter::Exit()
{
	UnWireSubs();

	uiRegistry->SetEnabled(config.keys.ringBack, false);
	uiRegistry->SetEnabled(config.keys.ringFront, false);
	uiRegistry->SetEnabled(config.keys.portrait, false);

	HideFatalText();
	HideLetters();

	rt = FatalDriveRuntime{};
}

void BattleFatalDrivePresenter::EnsureInstances()
{
	uiRegistry->Ensure(config.keys.ringBack);
	uiRegistry->Ensure(config.keys.ringFront);
	uiRegistry->Ensure(config.keys.portrait);

	uiRegistry->Ensure(config.keys.txtFatal);
	uiRegistry->Ensure(config.keys.txtRedLight);
	uiRegistry->Ensure(config.keys.txtDrive);

	uiRegistry->Ensure(config.keys.bgLeft);
	uiRegistry->Ensure(config.keys.bgRight);

	uiRegistry->SetEnabled(config.keys.ringBack,  true);
	uiRegistry->SetEnabled(config.keys.ringFront, true);
	uiRegistry->SetEnabled(config.keys.portrait,  true);

	uiAnimSys->SetScale(config.keys.ringBack,  config.layout.scale.x, config.layout.scale.y);
	uiAnimSys->SetScale(config.keys.ringFront, config.layout.scale.x, config.layout.scale.y);
	uiAnimSys->SetScale(config.keys.portrait,  config.layout.scale.x, config.layout.scale.y);

	uiAnimSys->SetOpacity(config.keys.ringBack, 1.f);
	uiAnimSys->SetOpacity(config.keys.ringFront, 1.f);

	uiAnimSys->SetOpacity(config.keys.bgLeft, 0.f);
	uiAnimSys->SetOpacity(config.keys.bgRight, 0.f);

	for (const auto& key : letterSpec.keys)
	{
		uiRegistry->Ensure(key);
		uiRegistry->SetEnabled(key, false);
	}
}

void BattleFatalDrivePresenter::StartActivation()
{
	rt.active = true;
	rt.progress = 0.f;
	rt.portraitShown = false;

	uiRegistry->SetEnabled(config.keys.ringBack, true);
	uiRegistry->SetEnabled(config.keys.ringFront, true);

	uiRegistry->SetFillRatioX(config.keys.ringFront, 0.f);

	uiAnimSys->SetOpacity(config.keys.ringFront, 0.f);
	uiAnimSys->SetOpacity(config.keys.ringBack, 0.f);
	uiAnimSys->PlayFadeOnce(config.keys.ringFront, 0.f, 1.f, 0.25f);
	uiAnimSys->PlayFadeOnce(config.keys.ringBack, 0.f, 1.f, 0.25f);

	ShowFatalText();
}

void BattleFatalDrivePresenter::Tick(float dt)
{
	if (registry.Get<InputService>().KeyDown(KEY::F))
		StartActivation();

	if (rt.active)
	{
		rt.progress += dt * 0.4f;
		if (rt.progress >= 1.f)
		{
			rt.progress = 1.f;
			rt.active = false;

			uiAnimSys->PlayFadeOnce(config.keys.ringFront, 1.0f, 0.0f, 0.4f);
			uiAnimSys->PlayFadeOnce(config.keys.ringBack, 1.0f, 0.0f, 0.4f);

			rt.portraitTimer = 0.4f;
			ShowPortrait();
		}
		uiRegistry->SetFillRatioX(config.keys.ringFront, rt.progress);
	}

	if (rt.portraitTimer > 0.f)
	{
		rt.portraitTimer -= dt;
		if (rt.portraitTimer <= 0.f)
		{
			uiRegistry->SetEnabled(config.keys.ringBack, false);
			uiRegistry->SetEnabled(config.keys.ringFront, false);
		}
	}

	if (rt.letters.active)
		TickLetterReveal(dt);

	if (rt.portraitShown)
	{
		if (!rt.portraitFadingOut)
		{
			rt.portraitTimer -= dt;
			if (rt.portraitTimer <= 0.f)
			{
				rt.portraitFadingOut = true;
				rt.portraitTimer = 0.8f;

				uiAnimSys->PlayFadeOnce(config.keys.portrait, 1.0f, 0.0f, 0.8f);
				uiAnimSys->PlayScaleOnce(config.keys.portrait, config.layout.scale.x * 1.0f, config.layout.scale.y * 1.0f,
					                                           config.layout.scale.x * 1.5f, config.layout.scale.y * 1.5f, 0.7f, UIEasing::EaseInOut);
				uiAnimSys->PlayFadeOnce(config.keys.bgLeft, 1.0f, 0.0f, 0.8f);
				uiAnimSys->PlayFadeOnce(config.keys.bgRight, 1.0f, 0.0f, 0.8f);

				FadeOutLetters();
			}
		}
		else
		{
			rt.portraitTimer -= dt;
			if (rt.portraitTimer <= 0.f)
			{
				uiAnimSys->SetOpacity(config.keys.portrait, 0.0f);
				uiAnimSys->SetScale(config.keys.portrait, config.layout.scale.x, config.layout.scale.y);
				uiAnimSys->SetOffSet(config.keys.portrait, 0.f, 0.f);

				uiAnimSys->SetOpacity(config.keys.bgLeft, 0.0f);
				uiAnimSys->SetOpacity(config.keys.bgRight, 0.0f);

				rt.portraitShown = false;
				rt.portraitFadingOut = false;
				rt.portraitTimer = 0.f;
			}
		}
	}
}

void BattleFatalDrivePresenter::ShowPortrait()
{
	if (rt.portraitShown) return;

	HideFatalText();
	BeginLetterReveal();

	const EntityID leader = timelineSys->GetLeader();
	const wstring fatalKey = dataSys->GetTextureKey(leader, UITextureSlot::FatalDrive);
	uiRegistry->SetWidgetTexture(config.keys.portrait, fatalKey);

	const float inDur  = 0.5f;
	const float hold   = 2.f;
	const float outDur = 0.5f;
	const float total = inDur + hold + outDur;
	const float driftX = -60.f;

	uiAnimSys->PlayFadeOnce(config.keys.bgLeft, 0.f, 1.f, inDur);
	uiAnimSys->PlayFadeOnce(config.keys.bgRight, 0.f, 1.f, inDur);

	uiAnimSys->SetScale(config.keys.portrait, config.layout.scale.x * 0.5f, config.layout.scale.y * 0.5f);
	uiAnimSys->SetOpacity(config.keys.portrait, 0.5f);
	uiAnimSys->SetOffSet(config.keys.portrait, 0.f, 0.f);

	uiAnimSys->PlayScaleOnce(config.keys.portrait, config.layout.scale.x * 0.5f, config.layout.scale.y * 0.5f, config.layout.scale.x * 1.0f, config.layout.scale.y * 1.0f, inDur);

	uiAnimSys->PlayFadeOnce(config.keys.portrait, 0.5f, 1.0f, inDur);
	uiAnimSys->PlaySlideOnce(config.keys.portrait, 0.f, 0.f, driftX, 0.f, total);

	rt.portraitShown = true;
	rt.portraitFadingOut = false;
	rt.portraitTimer = hold;
}

void BattleFatalDrivePresenter::WireSubs()
{
	if (wired) return;

	listenerIds.push_back(
		eventBus->Subscribe(BattleBusEventType::TacticMaxBlinkOn,
			[&](const BattleEvent& e) { OnBusEvent(e); })
	);

	listenerIds.push_back(
		eventBus->Subscribe(BattleBusEventType::TacticMaxBlinkOff,
			[&](const BattleEvent& e) { OnBusEvent(e); })
	);

	listenerIds.push_back(
		eventBus->Subscribe(BattleBusEventType::SessionEnded,
			[&](const BattleEvent& e) { OnBusEvent(e); })
	);

	wired = true;
}

void BattleFatalDrivePresenter::UnWireSubs()
{
	if (!wired) return;
	for (auto id : listenerIds) eventBus->Unsubscribe(id);
	listenerIds.clear();
	wired = false;
}


void BattleFatalDrivePresenter::OnBusEvent(const BattleEvent& event)
{
	switch (event.eventType)
	{
	case BattleBusEventType::TacticMaxBlinkOn:
		StartActivation();
		break;

	case BattleBusEventType::TacticMaxBlinkOff:
		rt.active = false;
		rt.progress = 0.f;
		uiRegistry->SetFillRatioX(config.keys.ringFront, 0.f);
		uiAnimSys->SetOpacity(config.keys.portrait, 0.f);
		uiAnimSys->SetOpacity(config.keys.bgLeft, 0.f);
		uiAnimSys->SetOpacity(config.keys.bgRight, 0.f);
		rt.portraitShown = false;
		HideFatalText();
		HideLetters();
		rt.letters.active = false;
		break;

	case BattleBusEventType::SessionEnded:
		HideFatalText();
		HideLetters();
		uiAnimSys->SetOpacity(config.keys.bgLeft, 0.f);
		uiAnimSys->SetOpacity(config.keys.bgRight, 0.f);
		rt.letters.active = false;
		Exit();
		break;

	default: break;
	}
}

void BattleFatalDrivePresenter::ShowFatalText()
{
	uiRegistry->SetEnabled(config.keys.txtFatal, true);
	uiRegistry->SetEnabled(config.keys.txtRedLight, true);
	uiRegistry->SetEnabled(config.keys.txtDrive, true);

	uiAnimSys->SetOpacity(config.keys.txtFatal, 1.f);
	uiAnimSys->SetOpacity(config.keys.txtRedLight, 1.f);
	uiAnimSys->SetOpacity(config.keys.txtDrive, 1.f);
}

void BattleFatalDrivePresenter::HideFatalText()
{
	uiRegistry->SetEnabled(config.keys.txtFatal, false);
	uiRegistry->SetEnabled(config.keys.txtRedLight, false);
	uiRegistry->SetEnabled(config.keys.txtDrive, false);
}

void BattleFatalDrivePresenter::BeginLetterReveal()
{
	HideLetters();

	rt.letters.Reset(letterSpec.keys.size(), letterSpec.revealDur, letterSpec.staggerFrac);
	rt.letters.active = true;

	StartLetterAt(0);
	rt.letters.started[0] = 1;
}

void BattleFatalDrivePresenter::TickLetterReveal(float dt)
{
	if (!rt.letters.active) return;

	rt.letters.elapsed += dt;

	for (int i = 0; i < (int)letterSpec.keys.size(); ++i)
	{
		if (rt.letters.started[i]) continue;

		const float needTime = i * rt.letters.stagger; 
		if (rt.letters.elapsed >= needTime)
		{
			StartLetterAt(i);
			rt.letters.started[i] = 1;
		}
	}

	bool allStarted = true;
	for (auto s : rt.letters.started) if (!s) { allStarted = false; break; }
	if (allStarted) rt.letters.active = false;
}

void BattleFatalDrivePresenter::StartLetterAt(int idx)
{
	if (idx < 0 || idx >= (int)letterSpec.keys.size()) return;

	const wstring& key = letterSpec.keys[(size_t)idx];

	uiRegistry->SetEnabled(key, true);
	uiAnimSys->SetOpacity(key, 0.0f);
	uiAnimSys->SetScale(key, 1.0f, 0.0f);
	uiAnimSys->SetOffSet(key, 0.f, 0.f);

	uiAnimSys->PlayScaleOnce( key, 1.0f, 0.0f, 1.0f, 1.0f, letterSpec.revealDur, letterSpec.inEase);
	uiAnimSys->PlayFadeOnce( key, 0.0f, 1.0f, letterSpec.revealDur, letterSpec.inEase );
	uiAnimSys->PlaySlideOnce( key, 0.f, 0.f, letterSpec.driftX, 0.f, letterSpec.driftDur, UIEasing::Linear);
}

void BattleFatalDrivePresenter::FadeOutLetters()
{
	for (const auto& key : letterSpec.keys)
	{
		uiAnimSys->PlayFadeOnce( key, 1.0f, 0.0f, letterSpec.outDur, letterSpec.outEase);
		uiAnimSys->PlayScaleOnce( key, 1.0f, 1.0f, letterSpec.outScale, letterSpec.outScale, letterSpec.outDur, letterSpec.outEase);
	}
}

void BattleFatalDrivePresenter::HideLetters()
{
	for (const auto& key : letterSpec.keys)
	{
		uiRegistry->SetEnabled(key, false);
		uiAnimSys->SetOpacity(key, 0.0f);
		uiAnimSys->SetScale(key, 1.0f, 0.0f);
		uiAnimSys->SetOffSet(key, 0.f, 0.f);
	}
}