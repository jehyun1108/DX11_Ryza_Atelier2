#pragma once

NS_BEGIN(Engine)

struct PlayerTacticState
{
	int  level    = 0;
	int  pips     = 0;
	bool maxBlink = false;
};

enum class TacticEventType
{
	Changed,
	LevelUp, 
	MaxBlinkOn,
	MaxBlinkOff
};

NS_END