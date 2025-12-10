#include "pch.h"
#include "LightSpawner.h"

LightSpawner::LightSpawner(SystemRegistry& registry) : registry(registry)
{
	spawner = &registry.Get<EntitySpawner>();
}

EntityHandles LightSpawner::SpawnDirectionalLight(const _float3& dirWorld, const _float3& diffuseRgb, const _float3& ambientRgb, const _float3& specRgb)
{
    LightProxy proxy{};
    proxy.type = ENUM(LIGHT::DIRECTIONAL);
    proxy.range = 1000.f; 
    proxy.spotAngle = XM_PI / 4.f;

    proxy.ambient = _float4{ ambientRgb.x, ambientRgb.y, ambientRgb.z, 1.f };
    proxy.diffuse = _float4{ diffuseRgb.x, diffuseRgb.y, diffuseRgb.z, 1.f };
    proxy.specular = _float4{ specRgb.x, specRgb.y, specRgb.z, 1.f };

    _float3 d = dirWorld;
    proxy.lightDir = _float4{ d.x, d.y, d.z, 0.f };
    proxy.lightPos = _float4{ 0.f, 0.f, 0.f, 1.f }; 

    auto light = spawner->NewEntity()
        .WithTf()                 
        .WithDirectionalLight()   
        .Build();

    return light;
}

EntityHandles LightSpawner::SpawnPointLight(const _float3& pos, float range, const _float3& diffuseRgb, const _float3& ambientRgb, const _float3& specRgb)
{
    LightProxy proxy{};
    proxy.type = ENUM(LIGHT::POINT);
    proxy.range = range;
    proxy.spotAngle = XM_PI / 4.f;

    proxy.ambient = _float4{ ambientRgb.x, ambientRgb.y, ambientRgb.z, 1.f };
    proxy.diffuse = _float4{ diffuseRgb.x, diffuseRgb.y, diffuseRgb.z, 1.f };
    proxy.specular = _float4{ specRgb.x, specRgb.y, specRgb.z, 1.f };

    proxy.lightPos = _float4{ pos.x, pos.y, pos.z, 1.f };
    proxy.lightDir = _float4{ 0.f, -1.f, 0.f, 0.f }; 

    auto light = spawner->NewEntity()
        .WithTf(TransformDesc{ .pos = pos })
        .WithPointLight(proxy)
        .Build();

    return light;
}

EntityHandles LightSpawner::SpawnSpotLight(const _float3& pos, const _float3& dirWorld, float range, float spotAngleRad, const _float3& diffuseRgb, const _float3& ambientRgb, const _float3& specRgb)
{
    LightProxy proxy{};
    proxy.type = ENUM(LIGHT::SPOT);
    proxy.range = range;
    proxy.spotAngle = spotAngleRad;

    proxy.ambient = _float4{ ambientRgb.x, ambientRgb.y, ambientRgb.z, 1.f };
    proxy.diffuse = _float4{ diffuseRgb.x, diffuseRgb.y, diffuseRgb.z, 1.f };
    proxy.specular = _float4{ specRgb.x, specRgb.y, specRgb.z, 1.f };

    proxy.lightPos = _float4{ pos.x, pos.y, pos.z, 1.f };
    proxy.lightDir = _float4{ dirWorld.x, dirWorld.y, dirWorld.z, 0.f };

    auto light = spawner->NewEntity()
        .WithTf(TransformDesc{ .pos = pos })
        .WithSpotLight(proxy)
        .Build();

    return light;
}
