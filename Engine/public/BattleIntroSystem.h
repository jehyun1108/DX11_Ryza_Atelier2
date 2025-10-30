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

	bool    TryQueryTargetPos(EntityID entity, _float3& outTargetWorld) const; // Session -> Target
	_float3 QueryCenterWorldOrDefault() const;                                 // Session -> Center
};

NS_END