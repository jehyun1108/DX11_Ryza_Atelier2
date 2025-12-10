#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL ParticleSystem : public EntitySystem<ParticleSpawnerInstance>
{
public:
	explicit ParticleSystem(SystemRegistry& registry) : EntitySystem(registry) {}
	Handle   CreateSpawner(EntityID owner, const ParticleSpawnData* spawnData, const _float3& worldPos);

	void OnBoot() override;
	void Tick(float dt);

	void SetSpawnerPos(Handle handle, const _float3& worldPos) { Get(handle)->worldPos = worldPos; }
	void DestroySpawner(Handle handle)                         { Destroy(handle); }

	void ExtractParticleSnapshot(ParticleSnapshot& out, const CameraProxy& cam);
	void SpawnBurst(const ParticleSpawnData& spawnData, const _float3& worldPos, int count, EffectHandle owner);

	void SetSpawnerBasis(Handle handle, const _float3& right, const _float3& up, const _float3& forward, bool useBasis);

	void SetSpawnerOwner(Handle handle, EffectHandle effectHandle);
	void KillByOwner(EffectHandle effectHandle);

private:
	static constexpr int maxParticles = 8192;
	vector<Particle>     particles;
	int                  aliveCount = 0;

private:
	RenderSystem* renderSys{};
};

NS_END