#pragma once

NS_BEGIN(Client)

class LightSpawner
{
public:
	explicit LightSpawner(SystemRegistry& registry);

    EntityHandles SpawnDirectionalLight(const _float3& dirWorld   = _float3{ 0.5f, -1.0f, 0.3f },
                                        const _float3& diffuseRgb = _float3{ 1.f, 1.f, 1.f  },
                                        const _float3& ambientRgb = _float3{ 0.3f, 0.3f, 0.3f },
                                        const _float3& specRgb    = _float3{ 0.3f, 0.3f, 0.3f });

    EntityHandles SpawnPointLight(const _float3& pos,
                                  float          range,
                                  const _float3& diffuseRgb,
                                  const _float3& ambientRgb = _float3{ 0.0f, 0.0f, 0.0f },
                                  const _float3& specRgb    = _float3{ 0.0f, 0.0f, 0.0f });

    EntityHandles SpawnSpotLight(const _float3& pos,
                                 const _float3& dirWorld,
                                 float          range,
                                 float          spotAngleRad,
                                 const _float3& diffuseRgb,
                                 const _float3& ambientRgb = _float3{ 0.0f, 0.0f, 0.0f },
                                 const _float3& specRgb    = _float3{ 0.0f, 0.0f, 0.0f });

private:
	SystemRegistry& registry;
	EntitySpawner*  spawner{};
};

NS_END