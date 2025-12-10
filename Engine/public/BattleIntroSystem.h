#pragma once

#include "BattleIntroData.h"

NS_BEGIN(Engine)

class ENGINE_DLL BattleIntroSystem : public EntitySystem<BattleIntroState>
{
public:
	explicit BattleIntroSystem(SystemRegistry& registry) : EntitySystem(registry) {}
	void     OnBoot() override;

	Handle   Create(EntityID owner, Handle animHandle, Handle tfHandle, const AnimProfile& profile);
	void     Update(float dt);

private:
	const wstring& ResolveClip(const AnimProfile& profile, AnimKey key) const;
	bool IsCurClipFinished(const BattleIntroState& state) const;
	
	void PlayKey(BattleIntroState& state, AnimKey key, ANIMTYPE type, float fadeDur);
	void NextStage(BattleIntroState& state, AnimKey nextKey, BattleIntroStage nextStage, ANIMTYPE type, float fadeDur);
	bool PlayNextIntroChain(BattleIntroState& state);

	pair<BattleTeam, int>     GetTeamSlot(EntityID entity) const;
	_float3                   GetFormationTarget(EntityID entity) const;
	_float2                   GetFormationFace(EntityID entity) const;
	_float3                   GetCenterWorld() const;

private:
	AnimatorSystem*        animator{};
	TransformSystem*       tfSys{};
	FacingForceService*    faceSrv{};
	BattleSessionSystem*   sessionSys{};
	BattleFormationSystem* formationSys{};
	AnimDataSystem*        animDataSys{};
	ActionAnimRegistry*    actionReg{};
};

NS_END