#pragma once

#include "BattleTargetData.h"

NS_BEGIN(Engine)

class ENGINE_DLL BattleTargetSystem : public EntitySystem<Target>, public IGuiRenderable
{
public:
	explicit BattleTargetSystem(SystemRegistry& registry);

	void Init();
	void OnUnitDowned(EntityID downed);

	EntityID Get(EntityID attacker) const;
	void     Ensure(EntityID owner);

	void     RenderGui(EntityID id);

private:
	bool             Alive(EntityID id) const { return true; }
	vector<EntityID> Opponents(BattleTeam team) const;
	EntityID         Pick(const vector<EntityID>& vec);
	void             Pair();
	void             FillRest(const vector<int>& order, BattleTeam team);
	void             Set(EntityID attacker, EntityID target);

private:
	mt19937 rng;
};

NS_END