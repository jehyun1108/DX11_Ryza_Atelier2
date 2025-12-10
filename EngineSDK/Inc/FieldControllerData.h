#pragma once

NS_BEGIN(Engine)

struct PartyMember
{
	array<EntityID, 3> members{ invalidEntity, invalidEntity, invalidEntity };
	int count = 0;

	template<typename Func>
	void ForEachValid(Func&& visit) const
	{
		for (int i = 0; i < count; ++i)
		{
			const EntityID entity = members[i];
			if (entity != invalidEntity)
				visit(i, entity);
		}
	}
};
struct FieldControllerState
{
	EntityID leader{};
	Handle   camTf{};
	
	bool prevJumpDown   = false;
	bool prevAttackDown = false;
};
struct BattleFormationParams
{
	float ringRadius     = 100.f;
	float startAngleDeg  = 0.f;
	bool  faceCenterSnap = true;
};
struct BattleRequest
{
	EntityID              leader{};
	optional<PartyMember> partyMembers{};
	_float3               centerHint{};
	BattleFormationParams params{};
};

NS_END