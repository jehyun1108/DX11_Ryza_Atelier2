#pragma once

NS_BEGIN(Engine)

struct ExecutionChainCursor
{
	int  curChainIdx = 0;
	int  curStageIdx = 0;
	bool isActive    = false;
};

struct ExecutionUnitRunTime
{
	CharacterID          character = CharacterID::Unknown;
	AnimContext          context   = AnimContext::Battle;
	ExecutionChainCursor cursor    = {};
	TimelineActionIntent activeIntent{};
};

NS_END