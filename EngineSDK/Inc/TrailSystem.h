#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL TrailSystem : public EntitySystem<TrailInstance>
{
public:
	explicit TrailSystem(SystemRegistry& registry) : EntitySystem(registry) {}
	void     OnBoot() override;

	Handle   Create(EntityID owner, const TrailDesc& desc);

	void   AddSample(Handle trailHandle, const _float3& tipPos);
	void   Tick(float dt);
	void   ExtractTrailSnapshot(TrailSnapshot& out, CameraProxy& cam);

private:
	_float  ComputePathLength(const TrailInstance& t) const;
	_float3 SamplePathPos(const TrailInstance& t, float u) const;

private:
	ParticleSystem* particleSys{};
};

NS_END
