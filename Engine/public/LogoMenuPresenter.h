#pragma once

#include "LogoMenuData.h"

NS_BEGIN(Engine)

class ENGINE_DLL LogoMenuPresenter : public ISystem
{ 
public:
	explicit LogoMenuPresenter(SystemRegistry& registry) : registry(registry){}
	void     OnBoot() override;

	void     Enter();
	void     Tick(float dt);
	void     Exit();

	void     SetCommandCallback(function<void(LogoMenuCommand)> cb) { onCommand = move(cb); }

private:
	void InitButtons();
	void TickButtons(float dt);
	void HandleClick(LogoButtonID id);
	bool HitTest(const LogoButton& btn) const;

private:
	vector<LogoButton>      buttons;
	wstring                 hoverBarKey  = L"menu_hoverbar";
	wstring                 pressAnyKey  = L"press_any_button_0";
	wstring                 titleBar     = L"title";
	wstring                 logobg       = L"logo_bg";

	LogoPhase               phase        = LogoPhase::PressAny;
	float                   pressAnyTime = 0.f;
	float                   hoverBarTime = 0.f;

	function<void(LogoMenuCommand)> onCommand;

private:
	SystemRegistry&         registry;
	UIAnimSystem*           uiAnimSys{};
	UIRegistry*             uiRegistry{};
	InputService*           input{};
	GameModeDirectorSystem* director{};
	UISystem*               uiSys{};
	ScreenFadeSystem*       fadeSys{};
	SoundSystem*            soundSys{};
};

NS_END