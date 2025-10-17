#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL Animator final : public Component
{
public:
	explicit Animator(Obj* owner, shared_ptr<Skeleton> skeleton, const vector<shared_ptr<AnimClip>>& animClips);

	virtual void LateUpdate(float dt) override;

	void Play(_uint layerIdx, const wstring& clipName, ANIMTYPE type = ANIMTYPE::LOOP);
	void PlayFade(_uint layerIdx, const wstring& clipName, float fadeSec, float targetWeight = 1.f, ANIMTYPE type = ANIMTYPE::LOOP);
	void Pause(_uint layerIdx);
	void Reset(_uint layerIdx);
	bool IsFinished(_uint layerIdx) const;

	void AddLayer(const vector<string>& maskedBoneNames = {});
	void SetLayerBlendWeight(_uint layerIdx, float weight);
	void SetLayerBlendType(_uint layerIdx, ANIMBLEND type);
	void SetPlaybackSpeed(_uint layerIdx, float speed);
	void SetLayerEnabled(_uint layerIdx, bool enabled);

	void SetLayerTime(_uint layerIdx, float tick);
	float GetClipDuration(const wstring& name) const;

	shared_ptr<AnimClip> GetClip(const wstring& name) const;
	vector<uint8_t> BuildMaskFromClip(const wstring& clipName, bool includeChildren = false) const;
	void SetLayerMask(uint32_t layerIdx, const vector<uint8_t>& mask);

	_uint GetBoneIdxByName(const string& boneName) const;
	const vector<_float4x4>& GetFinalMatrices() const { return finalMatrices; }
	_uint GetClipCount() const;
	_uint GetLayerCount() const;
	vector<wstring> GetClipNames() const;
	_mat GetBoneWorldMat(const string& boneName) const;
	_mat GetBoneWorldMat(_uint boneIdx) const;
	virtual void RenderGui() override;
	virtual COMPONENT GetType() const { return COMPONENT::ANIMATOR; }

private:
	// 파이프라인 단게
	void BaseSRT(vector<_vec>& outScale, vector<_vec>& outRot, vector<_vec>& outTrans);
	void BlendSRT(vector<_vec>& inoutScale, vector<_vec>& inoutRot, vector<_vec>& inoutTrans);
	void BuildLocalSRT(vector<_vec>& scale, vector<_vec>& rot, vector<_vec>& trans);
	void SetFinalMatrices();

	void SampleSRT(_uint boneIdx, const AnimLayer& layer, _vec& outScale, _vec& outRot, _vec& outTrans) const;
	void SampleSRTAt(_uint boneIdx, const AnimLayer& layer, float time, _vec& outScale, _vec& outRot, _vec& outTrans) const;
	void SampleAddDelta(_uint boneIdx, const AnimLayer& layer, _vec& outDeltaScale, _vec& outDeltaRot, _vec& outDeltaTrans) const;

	template<typename T>
	pair<uint16_t, uint16_t> FindKeyInterval(const vector<Keyframe<T>>& keys, float time, uint16_t hint)const
	{
		const uint16_t n = (uint16_t)keys.size();
		if (n <= 1) return { 0, 0 };
		if (time <= keys.front().time) return { 0, 0 };
		if (time >= keys.back().time) return { (uint16_t)(n - 1), (uint16_t)(n - 1) };

		// 전방 힌트 선형 스캔 (O(1))
		uint16_t i = hint;
		while (i + 1 < n && !(time <= keys[i + 1].time)) ++i;
		if (i + 1 < n && time >= keys[i].time && time <= keys[i + 1].time)
			return { i, (uint16_t)(i + 1) };

		// 실패 시 이진탐색
		uint16_t lo = 0, hi = n - 1;
		while (lo + 1 < hi)
		{
			uint16_t mid = (lo + hi) / 2;
			(keys[mid].time <= time) ? lo = mid : hi = mid;
		}
		return { lo, hi };
	}

	static float Clamp01(float x) { return max(0.f, min(1.f, x)); }

private:
	shared_ptr<Skeleton> skeleton;
	map<wstring, shared_ptr<AnimClip>> clipsByName;

	vector<AnimLayer> layers;

	// 평가 버퍼(SoA)
	vector<_vec> baseScale;
	vector<_vec> baseRot;
	vector<_vec> baseTrans;

	vector<_vec> blendedScale;
	vector<_vec> blendedRot;
	vector<_vec> blendedTrans;

	vector<_float4x4> finalMatrices;

	Transform* tf{};
	_uint boneCount = 0;
};

NS_END
