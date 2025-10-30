#pragma once

#include "BattleTransitionData.h"

NS_BEGIN(Engine)

class ENGINE_DLL BattleTransitionSystem 
{
public:
	explicit BattleTransitionSystem(SystemRegistry& registry) : registry(registry) {}
	void     BeginBattle(const BattleRequest& request);

private:
	PartyMember   ResolveParty(const optional<PartyMember>& party, EntityID leader) const;
	vector<float> ComputeAngleDeg(int memberCount, float startAngleDeg) const;
	_float3       ComputeSlotPos(const _float3& center, float angleDeg, float ringRadius) const;

	bool        TryResolveAnim(EntityID entity, Handle& outAnim, Handle& outTf) const;
	AnimProfile ResolveProfile(EntityID entity) const;

private:
	SystemRegistry& registry;
};

NS_END