#pragma once

NS_BEGIN(Client)

class EnemySpawner
{
public:
	explicit EnemySpawner(SystemRegistry& registry);

public:
	EntityHandles         SpawnAngel(const _float3& pos);
	vector<EntityHandles> SpawnAngels(const vector<_float3>& positions);

private:
	SystemRegistry&      registry;
	EntitySpawner*       spawner{};
	CharacterDataSystem* dataSys{};
};

NS_END