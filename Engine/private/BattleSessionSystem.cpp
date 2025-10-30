#include "Enginepch.h"

static inline vector<float> ComputeAnglesDeg(int memberCount, float startDeg)
{
	vector<float> angles;
	angles.reserve(memberCount);
	if (memberCount == 1) angles.push_back(startDeg);
	else if (memberCount == 2) angles = { startDeg, startDeg - 120.f };
	else angles = { startDeg, startDeg - 120.f, startDeg + 120.f };
	return angles;
}

static inline _float3 ComputeRingSlot(const _float3& center, float angleDeg, float radius)
{
	const float rad = XMConvertToRadians(angleDeg);
	const float x = sinf(rad) * radius;
	const float z = cosf(rad) * radius;
	return _float3{ center.x + x, center.y, center.z + z };
}
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------
void BattleSessionSystem::BeginSession(const BattleParty& allies, const BattleEnemies& enemies, const _float3& centerWorld, const BattleSessionConfig& config)
{
	sessionState = BattleSessionState{};
	BattleSessionState& state = *sessionState;
	state.phase            = BattlePhase::Intro;
	state.elapsed          = 0.f;
	state.allies           = allies;
	state.enemies          = enemies;
	state.centerWorld      = centerWorld;
	state.config           = config;
	state.victoryCondition = BattleVictoryCondition{};
	state.isIntroFinished  = false;
	state.isResultFinished = false;
	state.leaderEntity     = (allies.memberCount > 0) ? allies.members[0] : invalidEntity;
	state.layout.centerWorld  = centerWorld;
	state.layout.spacing      = state.spawnConfig.spacing;
	
	state.alliesReadyCount    = 0;
	state.introReadyEventSent = false;
	state.alliesReadyEntities.clear();

	AssignSlots();
	ComputeTargetsFromInit();
	PushEvent({ BattleSessionEventType::SessionBegan });
}

void BattleSessionSystem::Update(float dt)
{
	if (!sessionState.has_value()) return;
	BattleSessionState& state = *sessionState;
	state.elapsed += dt;

	switch (state.phase)
	{
	case BattlePhase::Intro:
		if (state.isIntroFinished)
		{
			state.phase = BattlePhase::Active;
			PushEvent({ BattleSessionEventType::SessionActivated });
		}
		break;

	case BattlePhase::Active:
		if (state.isResultFinished)
		{
			state.phase = BattlePhase::Exit;
			PushEvent({ BattleSessionEventType::SessionResultDecided });
		}
		break;

	case BattlePhase::Result:
		break;

	case BattlePhase::Exit:
		break;
	}
}

void BattleSessionSystem::EndSession()
{
	if (!sessionState.has_value()) return;
	PushEvent({ BattleSessionEventType::SessionEnded });
	sessionState.reset();
	eventQueue.clear();
}

void BattleSessionSystem::SetCenter(const _float3& newCenterWorld)
{
	if (!sessionState.has_value()) return;
	sessionState->layout.centerWorld = newCenterWorld;
	ComputeTargetsFromInit();
}

void BattleSessionSystem::SetSpacing(float newSpacing)
{
	if (!sessionState.has_value()) return;
	sessionState->layout.spacing = newSpacing;
	ComputeTargetsFromInit();
}

void BattleSessionSystem::SetAllyStartAngleDeg(float deg)
{
	if (!sessionState.has_value()) return;
	sessionState->layout.allyStartAngleDeg = deg;
	ComputeTargetsFromInit();
}

bool BattleSessionSystem::TryGetIntroTargetPos(EntityID entity, _float3& outWorldPos) const
{
	if (!sessionState.has_value()) return false;
	const BattleSessionState& state = *sessionState;
	auto itTeam = state.slots.teamByEntity.find(entity);
	if (itTeam == state.slots.teamByEntity.end())
		return false;

	const BattleTeam team = itTeam->second;

	if (team == BattleTeam::Ally)
	{
		auto itSlot = state.slots.allySlotIdxByEntity.find(entity);
		if (itSlot == state.slots.allySlotIdxByEntity.end())
			return false;

		int slotIdx = itSlot->second;
		if (slotIdx < 0 || slotIdx >= state.layout.allySlotUsed)
			return false;

		outWorldPos = state.layout.allyTargetWorld[slotIdx];
		return true;
	}
	else if (team == BattleTeam::Enemy)
	{
		auto itSlot = state.slots.enemySlotIdxByEntity.find(entity);
		if (itSlot == state.slots.enemySlotIdxByEntity.end())
			return false;

		int slotIdx = itSlot->second;
		if (slotIdx < 0 || slotIdx >= state.layout.enemySlotUsed)
			return false;

		outWorldPos = state.layout.enemyTargetWorld[slotIdx];
		return true;
	}
	return false;
}

bool BattleSessionSystem::TryGetTeam(EntityID entity, BattleTeam& outTeam) const
{
	if (!sessionState.has_value()) return false;
	const BattleSessionState& state = *sessionState;
	auto it = state.slots.teamByEntity.find(entity);
	if (it == state.slots.teamByEntity.end())
		return false;

	outTeam = it->second;
	return true;
}

bool BattleSessionSystem::TryGetSlotIdx(EntityID entity, int& outSlotIdx) const
{
	if (!sessionState.has_value()) return false;
	const BattleSessionState& state = *sessionState;
	auto itTeam = state.slots.teamByEntity.find(entity);
	if (itTeam == state.slots.teamByEntity.end())
		return false;

	if (itTeam->second == BattleTeam::Ally)
	{
		auto it = state.slots.allySlotIdxByEntity.find(entity);
		if (it == state.slots.allySlotIdxByEntity.end())
			return false;
		outSlotIdx = it->second;
		return true;
	}
	else if (itTeam->second == BattleTeam::Enemy)
	{
		auto it = state.slots.enemySlotIdxByEntity.find(entity);
		if (it == state.slots.enemySlotIdxByEntity.end())
			return false;
		outSlotIdx = it->second;
		return true;
	}
	return false;
}

bool BattleSessionSystem::TryGetIntroFaceXZ(EntityID entity, _float2& outDirXZ) const
{
	if (!sessionState.has_value()) return false;
	const BattleSessionState& state = *sessionState;

	BattleTeam team;
	int slotIdx;
	if (!TryGetTeam(entity, team)) return false;
	if (!TryGetSlotIdx(entity, slotIdx)) return false;

	if (team == BattleTeam::Ally)
	{
		if (slotIdx < 0 || slotIdx >= state.layout.allySlotUsed) return false;
		outDirXZ = state.layout.allyFaceDirXZ[slotIdx];
		return true;
	}
	else if (team == BattleTeam::Enemy)
	{
		if (slotIdx < 0 || slotIdx >= state.layout.enemySlotUsed) return false;
		outDirXZ = state.layout.enemyFaceDirXZ[slotIdx];
		return true;
	}
	return false;
}

void BattleSessionSystem::ReportIntroReady(EntityID entity)
{
	if (!sessionState.has_value() || entity == invalidEntity) return;
	BattleSessionState& state = *sessionState;

	BattleTeam team;
	if (!TryGetTeam(entity, team)) return;

	if (team == BattleTeam::Ally)
	{
		if (state.alliesReadyEntities.find(entity) != state.alliesReadyEntities.end()) return;

		state.alliesReadyEntities.insert(entity);
		state.alliesReadyCount = min(state.alliesReadyCount + 1, state.allies.memberCount);

		if (!state.introReadyEventSent && state.alliesReadyCount >= state.allies.memberCount)
		{
			state.isIntroFinished = true;
			state.introReadyEventSent = true;
			PushEvent({ BattleSessionEventType::IntroReady });
		}
	}
}

void BattleSessionSystem::ReportResultDecided()
{
	if (!sessionState.has_value()) return;
	sessionState->isResultFinished = true;
}

void BattleSessionSystem::AssignSlots()
{
	if (!sessionState.has_value()) return;
	BattleSessionState& state = *sessionState;

	state.slots.allySlotIdxByEntity.clear();
	state.slots.enemySlotIdxByEntity.clear();
	state.slots.teamByEntity.clear();

	int allyUsed = 0;
	for (int i = 0; i < state.allies.memberCount && allyUsed < 3; ++i)
	{
		EntityID allyEntity = state.allies.members[i];
		if (allyEntity == invalidEntity) continue;

		state.slots.allySlotIdxByEntity[allyEntity] = allyUsed;
		state.slots.teamByEntity[allyEntity] = BattleTeam::Ally;
		++allyUsed;
	}
	state.layout.allySlotUsed = allyUsed;

	int enemyUsed = 0;
	for (int i = 0; i < state.enemies.memberCount && enemyUsed < 3; ++i)
	{
		EntityID enemyEntity = state.enemies.members[i];
		if (enemyEntity == invalidEntity) continue;

		state.slots.enemySlotIdxByEntity[enemyEntity] = enemyUsed;
		state.slots.teamByEntity[enemyEntity] = BattleTeam::Enemy;
		++enemyUsed;
	}
	state.layout.enemySlotUsed = enemyUsed;
}

void BattleSessionSystem::ComputeTargetsFromInit()
{
	if (!sessionState.has_value()) return;
	BattleSessionState& state = *sessionState;

	const _float3 center     = state.layout.centerWorld;
	const float   ringRadius = state.layout.spacing;
	const float   startDeg   = state.layout.allyStartAngleDeg;

	const int allyUsed = state.layout.allySlotUsed;
	const vector<float> allyAngles = ComputeAnglesDeg(allyUsed, startDeg);
	for (int i = 0; i < allyUsed; ++i)
		state.layout.allyTargetWorld[i] = ComputeRingSlot(center, allyAngles[i], ringRadius);
	for (int i = allyUsed; i < 3; ++i)
		state.layout.allyTargetWorld[i] = _float3{};

	const int enemyUsed = state.layout.enemySlotUsed;
	for (int i = 0; i < enemyUsed; ++i)
	{
		const float enemyDeg = (i < allyUsed) ? (allyAngles[i] + 180.f) : (startDeg + 180.f);
		state.layout.enemyTargetWorld[i] = ComputeRingSlot(center, enemyDeg, ringRadius);
	}
	for (int i = enemyUsed; i < 3; ++i)
		state.layout.enemyTargetWorld[i] = _float3{};

	// 3) FaceDir: 
	const int pairCount = min(allyUsed, enemyUsed);
	for (int i = 0; i < pairCount; ++i)
	{
		const _float3 allyPos    = state.layout.allyTargetWorld[i];
		const _float3 enemyPos   = state.layout.enemyTargetWorld[i];
		const _float2 allyDirXZ  = Utility::Normalize(enemyPos.x - allyPos.x, enemyPos.z - allyPos.z);
		const _float2 enemyDirXZ = _float2{ -allyDirXZ.x, -allyDirXZ.y };

		state.layout.allyFaceDirXZ[i]  = allyDirXZ;
		state.layout.enemyFaceDirXZ[i] = enemyDirXZ;
	}
}