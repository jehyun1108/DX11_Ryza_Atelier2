#pragma once

NS_BEGIN(Engine)

using ClipTable = unordered_map<wstring, const AnimClip*>;

struct ENGINE_DLL AnimLayerData
{
	// 재생 상태
	const AnimClip* clip;
	float curTime{};
	float playbackSpeed = 1.f;
	ANIMTYPE  playType  = ANIMTYPE::LOOP;
	ANIMBLEND blendType = ANIMBLEND::OVERRIDE;
	float blendWeight   = 1.f;

	// Fade
	float fadeTime{};
	float fadeElapsed{};
	float targetWeight = 1.f;
	bool  isPaused = false;
	bool  isEnabled = true;

	vector<uint16_t> lastPos, lastRot, lastScale;
	vector<uint8_t> mask;
};

struct ENGINE_DLL AnimData
{
	Skeleton* skeleton{};
	const ClipTable* clips{};
	Handle   transform{};
	_uint    boneCount{};

	vector<AnimLayerData> layers;

	vector<_vec> baseScale, baseRot, baseTrans;
	vector<_vec> blendScale, blendRot, blendTrans;

	vector<_float4x4> finalMatrices;
};

NS_END