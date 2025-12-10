#include "pch.h"
#include "SkyboxSpawner.h"

SkyboxSpawner::SkyboxSpawner(SystemRegistry& registry) : registry(registry)
{
	spawner = &registry.Get<EntitySpawner>();

}

EntityHandles SkyboxSpawner::SpawnSkybox(const wstring& tag, const wstring& modelKey, SkyTextureType type, bool attachToCam, float uniformScale, float baseYawRad, float rotSpeed, bool setActive)
{
    auto sky = spawner->NewEntity()
        .WithTf()
        .WithLayer(LayerUtil::LayerBit(LAYER::SKYBOX))
        .WithTag(Utility::ToString(tag))   
        .WithSkybox(modelKey, type, attachToCam, uniformScale, baseYawRad, rotSpeed, setActive)
        .Build();

    return sky;
}

EntityHandles SkyboxSpawner::SpawnNightSky()
{
    return SpawnSkybox(L"nightsky", L"nightsky");
}
