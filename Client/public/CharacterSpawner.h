#pragma once

NS_BEGIN(Client)

class CharacterSpawner
{
public:
	explicit CharacterSpawner(SystemRegistry& registry);

public:
	EntityHandles SpawnRyza(const _float3& pos);
	EntityHandles SpawnKlaudia(const _float3& pos);
	EntityHandles SpawnPatricia(const _float3& pos);


private:
	SystemRegistry&      registry;
	EntitySpawner*       spawner{};
	CharacterDataSystem* dataSys{};
	SocketSystem*        socket{};
};

NS_END