#pragma once

namespace Engine
{
	struct ObjData
	{
		_float4x4 world;
		_float4   color{};
		_float4x4 invWorld;

		float   outLinePixels{};
		_float2 vpSize{};
		float   padding{};
	};

	struct VertexColor
	{
		_float3 pos;
		_float4 color;
	};

	struct Vertex_PUV
	{
		_float3 pos;
		_float2 uv;
	};

	struct Vertex_PNU
	{
		_float3 pos;
		_float3 normal;
		_float2 uv;
	};

	struct Vertex_PNUTAN
	{
		_float3 pos;
		_float3 normal;
		_float2 uv;
		_float4 tangent;
	};

	struct Vertex_Anim
	{
		_float3 pos;
		_float3 normal;
		_float2 uv;
		_float4 tangent;

		_uint  boneIndices[4]{};
		_float boneWeights[4]{};
	};

	struct BoneMatrices
	{
		const _float4x4* data{};
		_uint count{};
	};

	template<typename T>
	struct Keyframe
	{
		float time;
		T value;
	};

	struct BoneAnim
	{
		string boneName;
		vector<Keyframe<_float3>> posKeys;
		vector<Keyframe<_float4>> rotKeys;
		vector<Keyframe<_float3>> scaleKeys;
	};

	struct AnimClip
	{
		string name;
		map<string, BoneAnim> boneAnims;
		float duration;
		float tickPerSec;
		ANIMTYPE type = ANIMTYPE::ONCE;
	};
	
    struct BoneInfo
    {
    	string    boneName;
    	_float4x4 invBindPose; // aiBone->mOffsetMatrix
		_float4x4 bindLocal; // aiNode->mTransformation
		int       parentIdx = -1;
		bool      isAnimated = true;
    };

    struct SkeletonInfo
    {
    	vector<BoneInfo> bones;
    	map<string, _uint> boneNameToIdx;
		int rootBoneIdx = -1;
    };

	struct AnimLayer
	{
		// 재생 클립
		shared_ptr<AnimClip> clip;
		int clipId = -1;

		// 시간/재생
		float curTime = 0.f;
		float playbackSpeed = 1.f;
		ANIMTYPE animType = ANIMTYPE::LOOP;
		bool isPaused = false;
		bool isEnabled = true;

		// 블렌딩
		float blendWeight = 1.f;
		float targetWeight = 1.f;
		float fadeTime = 0.f;
		float fadeElapsed = 0.f;
		ANIMBLEND blendType = ANIMBLEND::OVERRIDE;

		// 마스크 본별: enable(1) / disable(0)
		vector<uint8_t> boneAffectMask;

		vector<uint16_t> lastPosKeyIdx;
		vector<uint16_t> lastRotKeyIdx;
		vector<uint16_t> lastScaleKeyIdx;
	};
}