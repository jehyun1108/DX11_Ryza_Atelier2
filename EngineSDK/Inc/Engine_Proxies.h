#pragma once

namespace Engine
{
	class Mesh;
	class Material;
	class Skeleton;

	struct BoneMatrices
	{
		const _float4x4* data{};
		_uint count{};
	};

	struct RenderProxy
	{
		_uint                  owner = 0;
		shared_ptr<Mesh>       mesh;
		shared_ptr<Material>   material;
		_float4x4              world;

		optional<BoneMatrices> boneMatrices;
		shared_ptr<Skeleton>   skeleton;

		const BoundingBox* ownerBoundingBox{};
		bool               isfirstBox = false;
	};

	struct LightProxy
	{
		int type = ENUM(LIGHT::DIRECTIONAL);
		_float3 padding;

		_float4 ambient = { 0.3f, 0.3f, 0.3f, 0.3f };
		_float4 diffuse = { 1.f, 1.f, 1.f, 1.f };
		_float4 specular = { 1.f, 1.f, 1.f, 1.f };
		_float4 lightPos;
		_float4 lightDir = { 0.f, -1.f, 0.f, 0.f };

		_float range = 100.f;
		_float spotAngle = XM_PI / 4.f;
		_float2 padding2;
	};

	struct CameraProxy
	{
		_float4x4 view;
		_float4x4 proj;
		_float4 camPos;
	};
}