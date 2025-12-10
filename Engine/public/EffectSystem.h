#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL EffectSystem : public ISystem
{
public:
	explicit EffectSystem(SystemRegistry& registry) : registry(registry) {}
	
	void OnBoot() override;
	void Tick(float dt);

	EffectHandle PlayAt(const wstring& key, const _float3& worldPos);
	EffectHandle PlayAttached(const wstring& key, EntityID attachOwner, const _float3& localOffset = _float3{});
	EffectHandle PlayTrail(const wstring& key, EntityID attachOwner, float duration, float startDelay = 0.f);

	void Stop(EffectHandle handle);
	bool IsAlive(EffectHandle handle) const;

	void RegisterArchetype(EffectArchetype& def);
	void RenderArchetypeGui(EffectArchetype& effect);
	void SetDebugTrailAlwaysOn(bool v) { debugTrailAlwaysOn = v; }
	void ApplyEmitterPreset(EffectEmitterDesc& emitter, EmitterShapePreset preset);

private:
	EffectInstance*        GetInstance(EffectHandle handle);
	const EffectInstance*  GetInstance(EffectHandle handle) const;
	EffectInstance&        NewInstance(const EffectArchetype* archetype);
	const EffectArchetype* FindArchetype(const wstring& key) const;

	void    UpdateInstance(EffectInstance& inst, _uint idx, float dt);
	void    UpdateAttachmentAndTrailDir(EffectInstance& inst, _float3& outTrailDir);
	void    UpdateEmitter(EffectInstance& inst, const EffectEmitterDesc& desc, EffectEmitterRuntime& rt, const _float3& trailDir, float t, float dt);
	void    UpdateParticleEmitter(EffectInstance& inst, const EffectEmitterDesc& desc, EffectEmitterRuntime& rt, bool inRange);
	void    UpdateTrailEmitter(EffectInstance& inst, const EffectEmitterDesc& desc, EffectEmitterRuntime& rt, const _float3& trailDir, bool
		inRange, float dt);
	void    FinishInstance(EffectInstance& inst, _uint idx);
	_float3 CalcEmitterBasePosWorld(const EffectInstance& inst, const EffectEmitterDesc& desc);
	bool    ComputeTrailTipFromCollider(const EffectInstance& inst, const EffectEmitterDesc& desc, _float3& outTip);

private:
	vector<EffectInstance> instances;
	vector<_uint>          freeList;
	_uint                  nextHandle = 1;
	bool                   debugTrailAlwaysOn = false;
	unordered_map<wstring, shared_ptr<EffectArchetype>> archetypes;

private:
	void RenderArchetypeHeader(EffectArchetype& effect); 
	void RenderEmitterSection(EffectArchetype& effect);
	void RenderSingleEmitter(EffectArchetype& effect, size_t idx, const char** curveNames,int curveCount, int& removeIdx, int& duplicateIdx, int& moveUpIdx, int& moveDownIdx);
	void ApplyEmitterCommands(EffectArchetype& effect, int removeIdx, int duplicateIdx, int moveUpIdx, int moveDownIdx);
	void RenderEventSection(EffectArchetype& effect);
	void DrawParticleGui(ParticleSpawnData& spawn, const char** curveNames, int curveCount);
	void DrawTrailGui(EffectEmitterDesc& emitter , const char** curveNames, int curveCount);

private:
	SystemRegistry&  registry;
	ParticleSystem*  particleSys{};
	TransformSystem* tfSys{};
	EntityMgr*       entityMgr{};
	AssetSystem*     assets{};
	TrailSystem*     trailSys{};
	CollisionSystem* collisionSys{};

	EntityID         effectOwner = 0u;
};

NS_END