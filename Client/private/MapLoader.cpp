#include "pch.h"
#include "MapLoader.h"

#include "NavMeshSystem.h"
#include "WorldSerializer.h"

void MapLoader::LoadResources(WorldSerializer* worldSys, NavMeshSystem* navSys)
{
	vector<EntityID> entities;
	string errorMsg;
	worldSys->LoadWorldFromFile(L"../bin/Resources/Map/map_251202.dat", entities, errorMsg);

	filesystem::path tmp{ L"../bin/Resources/Map/navmesh_5.nav" };
	navSys->Load(tmp);
}
