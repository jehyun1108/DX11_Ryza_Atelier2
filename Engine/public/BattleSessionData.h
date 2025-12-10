#pragma once

NS_BEGIN(Engine)

enum class BattleSessionEventType
{
	IntroReady , SessionBegan, SessionActivated, SessionResultDecided, SessionEnded
};
struct BattleSide
{
	array<EntityID, 3> members{ 0u, 0u, 0u };
	int memberCount = 0;

	template<typename Func>
	void ForEachValid(Func&& visit) const
	{
		for (int memberIndex = 0; memberIndex < memberCount; ++memberIndex)
		{
			EntityID entity = members[memberIndex];
			if (entity != 0u)
				visit(memberIndex, entity);
		}
	}
};
using BattleParty   = BattleSide;
using BattleEnemies = BattleSide;
struct BattleSlotAssignment
{
	unordered_map<EntityID, int> allySlotIdxByEntity;
	unordered_map<EntityID, int> enemySlotIdxByEntity;
	unordered_map<EntityID, BattleTeam> teamByEntity; // 팀 체크 빠르게
};
struct BattleSessionEvent
{
	BattleSessionEventType type = BattleSessionEventType::SessionBegan;
};
struct BattleVictoryCondition
{
	bool allyMustSurvive  = true;
	bool enemyAllDefeated = true;
};
struct BattleSessionConfig
{
	float baseGaugeFillPerSec = 1.0f;
	int   maxSimultaneousActionsPerTeam = -1;
};
struct BattleSessionState
{
	BattlePhase phase = BattlePhase::Intro;
	float       elapsed = 0.f;

	BattleParty   allies{};
	BattleEnemies enemies{};

	BattleSessionConfig    config{};
	BattleVictoryCondition victoryCondition{};

	bool isIntroFinished  = false;
	bool isResultFinished = false;

	EntityID leaderEntity = 0u;

	BattleSlotAssignment    slots{};

	int  alliesReadyCount    = 0;
	bool introReadyEventSent = false;
	unordered_set<EntityID> alliesReadyEntities;

	_float3 centerWorld{};
};

NS_END