#pragma once

#include "BattleAttributeData.h"

NS_BEGIN(Engine)

class ENGINE_DLL BattleAttributeSystem : public ISystem
{
public:
	explicit BattleAttributeSystem(SystemRegistry& registry) : registry(registry) {}
	void     OnBoot() override;

	void     InitForSession(const BattleParty& allies, const BattleEnemies& enemies);
	void     EndSession();
	void     Tick(float dt);

	const HPState& GetHp(EntityID entity)        const { return RequireHp(entity); }
	float          GetHpRatio01(EntityID entity) const;
	float          GetStunRatio01(EntityID entity) const; 
	void           ApplyDamage(EntityID entity, int dmg);
	void           Heal(EntityID entity, int amount);
	void           SetMaxHp(EntityID entity, int maxHp);

	const StunState& GetStun(EntityID entity) const    { return RequireStun(entity); }
	void             SetStun(EntityID entity, const StunState& inState);
	void             AddStun(EntityID entity, float amount);
	bool             IsStunFull(EntityID entity) const { return RequireStun(entity).full; }
	void             ApplyHit(EntityID target, int dmg, float stun);

	void  Reserve(size_t n);
	void  SetConfg(const BattleAttributeConfig& cfg) { config = cfg; }
	const BattleAttributeConfig& GetConfig() const   { return config; }

private:
	HPState&         RequireHp(EntityID entity);
	const HPState&   RequireHp(EntityID entity) const;
	StunState&       RequireStun(EntityID entity);
	const StunState& RequireStun(EntityID entity) const;
	void             InitSide(const BattleSide& side);

private:
	BattleAttributeConfig              config{};
	unordered_map<EntityID, HPState>   hpByEntity;
	unordered_map<EntityID, StunState> stunByEntity;

private:
	SystemRegistry&      registry;
	CharacterDataSystem* dataSys{};
	BattleEventBus*      eventBus{};
	BattleSessionSystem* sessionSys{};
};

NS_END