#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL NavMeshSystem : public ISystem
{
public:
	explicit NavMeshSystem(SystemRegistry& registry) : registry(registry) {}

	void  ClearAll();
	bool  PushPointFromPick(const _float3& hitPos, const _float3& hitNormal);
	void  MoveVertex(_uint vid, const _float3& newPos);
	const NavigationData& GetData() const { return data; }

	bool Save(filesystem::path& path);
	bool Load(filesystem::path& path);
	void BuildDebugLines(vector<VertexColor>& out) const;
	void BuildDebugTriangles(vector<VertexColor>& out) const;

	bool DeleteLastTriangle();
	bool UndoLastPoint();

	bool RaycastDown(const _float3& origin, float maxDist, _float3& outHitPos, _float3& outNormal) const;
	bool SampleHeight(const _float3& pos, _float3& outPos, _float3& outNormal) const;

private:
	_uint AppendVertex(const _float3& p);
	void  FindNearestEdge(const _float3& p, _uint& outI, _uint& outJ) const;
	void  MakeTriangleWithWinding(_uint k, _uint i, _uint j, const _float3& hitNormals);
	_uint FindSnapVertex(const _float3& p, float radius) const;
	void  AppendCircle(vector<VertexColor>& out, const _float3& color, float r, _uint seg, const _float4& col) const;

	bool FindNearestPointEdge(const _float3& p, float radius, _float3& outPoint) const;

private:
	NavigationData data{};
	float snapRadius = 50.f;

private:
	SystemRegistry& registry;
};

NS_END