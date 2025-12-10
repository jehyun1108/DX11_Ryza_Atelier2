#pragma once

NS_BEGIN(Engine)
// 매 프레임 호출: ECS 수집 -> 큐 구성 -> 씬 스냅샷 채우기
class ENGINE_DLL RenderSystem : public ISystem
{
public:
	explicit RenderSystem(SystemRegistry& registry) : registry(registry) {}
	void     OnBoot() override;
	void     BuildScene(RenderScene& out);

	float CalcCamDist(const _float4x4& world, const CameraProxy& cam) const;
	float CalcCamDist(const _float3&   pos,   const CameraProxy& cam) const;

private:
	bool  FrustumCulling(const BoundingBox& worldAABB, const CameraProxy& cam) const;

private:
	unordered_map<const Material*, _uint> materialIdMap;
	unordered_map<const Mesh*,     _uint>     meshIdMap;
	_uint meshId     = 1;
	_uint materialId = 1;

private:
	SystemRegistry&  registry;
	CameraSystem*    camSys{};
	LightSystem*     lightSys{};
	SkyboxSystem*    skySys{};
	UISystem*        uiSys{};
	ModelSystem*     modelSys{};
	TransformSystem* tfSys{};
	LayerSystem*     layerSys{};
	AnimatorSystem*  animator{};
	CollisionSystem* collisionSys{};
	UIMinimapSystem* miniSys{};
	ParticleSystem*  particleSys{};
	TrailSystem*     trailSys{};
	EffectSystem*    effectSys{};
};

NS_END