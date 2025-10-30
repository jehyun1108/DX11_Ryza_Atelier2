#pragma once

NS_BEGIN(Engine)

enum class BattleIntroStage
{ 
	IntroStart,
	RunStart, RunLoop, RunEnd, 
	BattleIdle,
	Finished 
};

struct BattleIntroState
{
	EntityID entity{};
	Handle   animHandle{};
	Handle   tfHandle{};
	AnimProfile profile{ CharacterID::Unknown, AnimContext::Battle };
	BattleIntroStage stage = BattleIntroStage::IntroStart;

	int     introStageIdx = -1;
	float   elasped = 0.f;
	wstring curClipName;

	bool    reached = false;
	bool    readyIdle = false;
};

NS_END