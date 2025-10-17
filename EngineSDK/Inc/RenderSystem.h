#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL RenderSystem
{
public:
	explicit RenderSystem(SystemRegistry& registry) : registry(registry) {}

	// �� ������ ȣ��: ECS ���� -> ť ���� -> �� ������ ä���
	void BuildScene(RenderScene& out);

private:
	bool  FrustumCulling(const BoundingBox& worldAABB, const CameraProxy& cam) const;
	float CalcCamDist(const _float4x4& world, const CameraProxy& cam) const;

private:
	SystemRegistry& registry;
	unordered_map<const Material*, _uint> materialIdMap;
	unordered_map<const Mesh*,     _uint>     meshIdMap;
	_uint meshId     = 1;
	_uint materialId = 1;
};

NS_END