#pragma once

NS_BEGIN(Engine)
struct FormationParams;
// 누가 싸우는지, 언제 시작/종료되는지, 리더/페이즈/규칙 관리
class ENGINE_DLL BattleSessionSystem : public ISystem
{
public:
	explicit BattleSessionSystem(SystemRegistry& registry) : registry(registry) {}
	void     OnBoot() override;

	void BeginSession(const BattleParty& allies, const BattleEnemies& enemies, const _float3& centerWorld,
		const BattleSessionConfig& cfg = BattleSessionConfig{});
	void BeginSession(const BattleParty& allies, const BattleEnemies& enemies, const _float3& centerWorld, const FormationParams& introParams, const FormationParams& battleParams, const BattleSessionConfig& cfg = BattleSessionConfig{});

	void Update(float dt);
	void EndSession();

	bool                      HasActiveSession() const { return sessionState.has_value(); }
	BattlePhase               GetPhase()         const { return sessionState ? sessionState->phase : BattlePhase::Exit; }
	void                      SetPhase(BattlePhase newPhase) { sessionState->phase = newPhase; }

	const BattleSessionState& GetState()   const { return *sessionState; }
	const BattleParty&        GetAllies()  const { return sessionState->allies; }
	const BattleEnemies&      GetEnemies() const { return sessionState->enemies; }

	BattleTeam                 GetTeam(EntityID entity)     const;
	int                        GetSlotIdx(EntityID entity)  const;
	pair<BattleTeam, int>      GetTeamSlot(EntityID entity) const;

	const vector<BattleSessionEvent>& PeekEvent() const { return eventQueue; }
	void                              ClearEvents()     { eventQueue.clear(); }

	void ReportIntroReady(EntityID entity);
	void ReportResultDecided();

private:
	void AssignSlots();
	void PushEvent(BattleSessionEvent event) { eventQueue.push_back(event); }

private:
	optional<BattleSessionState> sessionState;
	vector<BattleSessionEvent>   eventQueue;

private:
	SystemRegistry& registry;
	BattleFormationSystem* formationSys{};
};

NS_END