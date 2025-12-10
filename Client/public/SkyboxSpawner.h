#pragma once

NS_BEGIN(Client)

class SkyboxSpawner
{
public:
	explicit SkyboxSpawner(SystemRegistry& registry);

    EntityHandles SpawnSkybox(const wstring& tag,
		                      const wstring& modelKey, 
		                      SkyTextureType type         = SkyTextureType::Equirect2D,
		                      bool           attachToCam  = true,
		                      float          uniformScale = 1200.f, 
		                      float          baseYawRad   = 0.f, 
		                      float          rotSpeed     = 0.02f, 
		                      bool           setActive    = true);
	
	EntityHandles SpawnNightSky();


private:
	SystemRegistry& registry;
	EntitySpawner*  spawner{};
};

NS_END