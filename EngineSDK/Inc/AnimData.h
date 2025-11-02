#pragma once

NS_BEGIN(Engine)

using ClipTable = unordered_map<wstring, const AnimClip*>;

struct ENGINE_DLL AnimLayerData
{
	const AnimClip* clip    = nullptr;
	float     curTime{};                  
	float     playbackSpeed = 1.0f;
	ANIMTYPE  playType      = ANIMTYPE::LOOP;
	ANIMBLEND blendType     = ANIMBLEND::OVERRIDE;
	float     blendWeight   = 0.f;        
	bool      isPaused      = false;
	bool      isEnabled     = true;

	float sectionStartTicks = 0.f; // 0 이면 클립시작
	float sectionEndTicks   = -1.f;

	vector<uint16_t> lastPos, lastRot, lastScale;
	vector<uint8_t>  mask;
};

struct CrossFadeState
{
	bool   isActive = false;
	_uint  fromLayerIndex = 0;
	_uint  toLayerIndex   = 4;
	float  durationSec    = 0.f;
	float  elapsedSec     = 0.f;

	float  fromStartWeight = 1.f;
	float  toStartWeight   = 0.f;

	bool   pendingSwap = false;

	wstring   toClipName{};
	ANIMTYPE  toAnimType = ANIMTYPE::ONCE;
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

	CrossFadeState cross{};
};

struct SectionInfo
{
	float startTicks = 0.f;
	float endTicks   = 0.f;
	float length     = 0.f;
};

NS_END