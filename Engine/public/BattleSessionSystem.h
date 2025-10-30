#pragma once

#include "BattleSessionData.h"

NS_BEGIN(Engine)
// 내부 시간/ 페이즈 관리, 이벤트 큐 유지
class ENGINE_DLL BattleSessionSystem
{
public:
	explicit BattleSessionSystem(SystemRegistry& registry) : registry(registry) {}

	void BeginSession(const BattleParty& allies, const BattleEnemies& enemies, 
		const _float3& centerWorld, const BattleSessionConfig& config = BattleSessionConfig{});
	void Update(float dt);
	void EndSession();
	// --------------------------------------------
	bool                      HasActiveSession() const { return sessionState.has_value(); }
	BattlePhase               GetPhase()         const { return sessionState ? sessionState->phase : BattlePhase::Exit; }
	const BattleSessionState* TryGetState()      const { return sessionState ? &(*sessionState) : nullptr; }
	EntityID             GetLeader()  const { return sessionState ? sessionState->leaderEntity : invalidEntity; }
	const BattleParty*   GetAllies()  const { return sessionState ? &sessionState->allies  : nullptr; }
	const BattleEnemies* GetEnemies() const { return sessionState ? &sessionState->enemies : nullptr; }
	// ---------------- 간격 ---------------------------------------
	_float3 GetCenter()  const { return sessionState ? sessionState->layout.centerWorld : _float3{}; }
	float   GetSpacing() const { return sessionState ? sessionState->layout.spacing : 300.f; }
	void    SetCenter(const _float3& newCenterWorld);
	void    SetSpacing(float newSpacing);
	void    SetAllyStartAngleDeg(float deg);
	//-------------- Intro ------------------------------
	bool TryGetIntroTargetPos(EntityID entity, _float3& outWorldPos) const;
	bool TryGetTeam(EntityID entity, BattleTeam& outTeam) const;
	bool TryGetSlotIdx(EntityID entity, int& outSlotIdx) const;
	bool TryGetIntroFaceXZ(EntityID entity, _float2& outDirXZ) const;
	// ------------- EventQueue --------------------------------
	const vector<BattleSessionEvent>& PeekEvent() const { return eventQueue; }
	void                              ClearEvents()     { eventQueue.clear(); }
	// -------------- Report ----------------------------
	void ReportIntroReady(EntityID entity);
	void ReportResultDecided();

private:
	// -------- 초기 슬롯 배정 (entity -> slotIdx)
	void AssignSlots();
	void ComputeTargetsFromInit();
	void PushEvent(const BattleSessionEvent& eventData) { eventQueue.push_back(eventData); }

private:
	SystemRegistry& registry;
	optional<BattleSessionState> sessionState;
	vector<BattleSessionEvent>   eventQueue;
};

NS_END