#pragma once

NS_BEGIN(Engine)

enum class BattleSessionEventType
{
	IntroReady , SessionBegan, SessionActivated, SessionResultDecided, SessionEnded
};

struct BattleSide
{
	array<EntityID, 3> members{ invalidEntity, invalidEntity, invalidEntity };
	int memberCount = 0;

	template<typename Func>
	void ForEachValid(Func&& visit) const
	{
		for (int memberIndex = 0; memberIndex < memberCount; ++memberIndex)
		{
			EntityID entity = members[memberIndex];
			if (entity != invalidEntity)
				visit(memberIndex, entity);
		}
	}
};

using BattleParty   = BattleSide;
using BattleEnemies = BattleSide;

struct BattleSpawnLayoutConfig
{
	array<_float3, 3> allyInitPos = {
		_float3{   0.f, 0.f, 1000.f},
		_float3{-150.f, 0.f, 1200.f},
		_float3{ 150.f, 0.f, 1200.f},
	};
	array<_float3, 3> enemyInitPos = {
		_float3{   0.f, 0.f,  0.f  },
		_float3{-150.f, 0.f, -150.f},
		_float3{ 150.f, 0.f, -150.f}
	};

	float spacing = 300.f;
	bool  faceCenterOnFinish = true;
};

struct BattleSessionLayout
{
	_float3 centerWorld = {};

	array<_float3, 3>  allyTargetWorld = { _float3{}, _float3{}, _float3{} };
	array<_float3, 3> enemyTargetWorld = { _float3{}, _float3{}, _float3{} };

	array<_float2, 3> allyFaceDirXZ  = { _float2{ 0, -1 }, _float2{ 0, -1 }, _float2{ 0, -1 } };
	array<_float2, 3> enemyFaceDirXZ = { _float2{ 0,  1 }, _float2{ 0,  1 }, _float2{ 0,  1 } };

	int allySlotUsed  = 0;
	int enemySlotUsed = 0;
	float spacing           = 300.f;
	float allyStartAngleDeg = 0.f;
};

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

struct BattleUnitRef
{
	EntityID   entity = invalidEntity;
	BattleTeam team   = BattleTeam::Neutral;
};

struct BattleVictoryCondition
{
	bool allyMustSurvive  = true;
	bool enemyAllDefeated = true;
};

struct BattleSessionConfig
{
	float baseGaugeFillPerSec = 1.0f;
	int maxSimultaneousActionsPerTeam = -1;
};

struct BattleSessionState
{
	BattlePhase phase = BattlePhase::Intro;
	float       elapsed = 0.f;

	BattleParty   allies{};
	BattleEnemies enemies{};

	_float3 centerWorld = {};

	BattleSessionConfig    config{};
	BattleVictoryCondition victoryCondition{};

	bool isIntroFinished  = false;
	bool isResultFinished = false;

	EntityID leaderEntity = invalidEntity;

	BattleSpawnLayoutConfig spawnConfig{};
	BattleSessionLayout     layout{};
	BattleSlotAssignment    slots{};

	int  alliesReadyCount    = 0;
	bool introReadyEventSent = false;
	unordered_set<EntityID> alliesReadyEntities;
};

NS_END