#pragma once

#include "AnimData.h"

NS_BEGIN(Engine)

class ENGINE_DLL AnimatorSystem : public EntitySystem<AnimData>, public IGuiRenderable
{
public:
	explicit AnimatorSystem(SystemRegistry& registry) : EntitySystem(registry) {}
	Handle Create(EntityID owner, Skeleton* skeleton, const ClipTable* clips, Handle transform, const vector<string>& baseMaskBones = {});

	void Update(float dt, TransformSystem& transformSys);

	void Play(Handle handle, _uint layerIdx, const wstring& clipName, ANIMTYPE type = ANIMTYPE::LOOP);
	void PlayFade(Handle handle, _uint layerIdx, const wstring& clipName, float fadeSec, float targetWeight = 1.f, ANIMTYPE type = ANIMTYPE::LOOP);

	void Pause(Handle handle, _uint layerIdx, bool toggle = true);
	void Reset(Handle handle, _uint layerIdx);

	void AddLayer(Handle handle, const vector<string>& maskedBoneNames = {});
	void SetLayerBlendWeight(Handle handle, _uint layerIdx, float weight);
	void SetLayerBlendType(Handle handle, _uint layerIdx, ANIMBLEND type);
	void SetPlaybackSpeed(Handle handle, _uint layerIdx, float speed);
	void SetLayerEnabled(Handle handle, _uint layerIdx, bool enabled);
	void SetLayerTime(Handle handle, _uint layerIdx, float tick);
	void SetLayerMask(Handle handle, _uint layerIdx, const vector<uint8_t>& mask);

	float            GetClipDuration(Handle handle, const wstring& clipName) const;
	vector<wstring>  GetClipNames(Handle handle) const;
	_uint            GetLayerCount(Handle handle) const;
	const _float4x4* GetBoneWorld(Handle handle, _uint boneIdx) const;
	_uint            GetBoneIdxByName(Handle handle, const string& boneName) const;
	const vector<_float4x4>* GetFinalMatrices(Handle handle) const;

	vector<uint8_t> BuildMaskFromClip(Handle handle, const wstring& clipName, bool includeChildren) const;
	void RenderGui(EntityID id) override;

private:
	const AnimClip* FindClip(const AnimData& anim, const wstring& clipName) const;

	void BaseSRT(AnimData& anim);
	void BlendSRT(AnimData& anim);
	void BuildLocalSRT(AnimData& anim);
	void SetFinalMatrices(AnimData& anim, const _float4x4& world);

	void SampleSRT(const AnimData& anim, _uint boneIdx, const AnimLayerData& layer, _vec& scale, _vec& rot, _vec& trans) const;
	void SampleSRTAt(const AnimData& anim, _uint boneIdx, const AnimLayerData& layer, float time, _vec& scale, _vec& rot, _vec& trans) const;
	void SampleAddDelta(const AnimData& anim, _uint boneIdx, const AnimLayerData& layer, _vec& deltaScale, _vec& deltaRot, _vec& deltaTrans) const;

	template<typename T>
	pair<uint16_t, uint16_t> FindKeyInterval(const vector<Keyframe<T>>& keys, float time, uint16_t hint) const;
};

template<typename T>
inline pair<uint16_t, uint16_t> AnimatorSystem::FindKeyInterval(const vector<Keyframe<T>>& keys, float time, uint16_t hint) const
{
	const uint16_t n = (uint16_t)keys.size();
	if (n <= 1) return { 0, 0 };
	if (time <= keys.front().time) return { 0, 0 };
	if (time >= keys.back().time)  return { (uint16_t)(n - 1), (uint16_t)(n - 1) };

	uint16_t i = hint;
	while (i + 1 < n && !(time <= keys[i + 1].time)) ++i;
	if (i + 1 < n && time >= keys[i].time && time <= keys[i + 1].time)
		return { i, (uint16_t)(i + 1) };

	uint16_t lo = 0, hi = n - 1;
	while (lo + 1 < hi)
	{
		uint16_t mid = (lo + hi) / 2;
		(keys[mid].time <= time) ? lo = mid : hi = mid;
	}
	return { lo, hi };
}

NS_END