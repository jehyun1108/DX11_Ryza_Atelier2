#pragma once

#include "BattleIntroData.h"

NS_BEGIN(Engine)

class ENGINE_DLL BattleIntroSystem : public EntitySystem<BattleIntroState>
{
public:
	explicit BattleIntroSystem(SystemRegistry& registry) : EntitySystem(registry) {}
	Handle   Create(EntityID owner, Handle animHandle, Handle tfHandle, const AnimProfile& profile);
	void     Update(float dt);

private:
	const wstring& ResolveClip(const AnimProfile& profile, AnimKey key) const;
	bool IsCurClipFinished(const BattleIntroState& state) const;
	
	void PlayKey(BattleIntroState& state, AnimKey key, ANIMTYPE type, float fadeDur);
	void NextStage(BattleIntroState& state, AnimKey nextKey, BattleIntroStage nextStage, ANIMTYPE type, float fadeDur);

	bool TryPlayNextIntroChain(BattleIntroState& state);

	bool    ResolveTeamSlot(EntityID entity, BattleTeam& outTeam, int& outSlot) const;
	bool    TryQueryFormationTarget(EntityID entity, _float3& outPos) const;
	bool    TryQueryFormationFace(EntityID entity, _float2& outDirXZ) const;
	_float3 QueryCenterWorld() const;
};

NS_END