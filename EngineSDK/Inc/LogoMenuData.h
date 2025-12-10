#pragma once

NS_BEGIN(Engine)

enum class LogoButtonID
{
	NewGame, LoadGame, Setting, Exit
};
enum class LogoPhase
{
	PressAny, Menu, FadingToField
};
enum class LogoMenuCommand
{
	NewGame, LoadGame, OpenSetting, ExitGame
};
struct LogoButton
{
	LogoButtonID  id;
	wstring       keyNormal;
	wstring       keyHover;
	UIWidgetState state = UIWidgetState::None;
};

NS_END