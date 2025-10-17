#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL MouthController final : public Component
{
public:
	explicit MouthController(Obj* owner, wstring mouthClip, uint32_t layerIdx = 2, float weight = 1.f, float speedScale = 1.f);

public:
	void SetWeight(float weight);
	void SetSpeed(float speed);
	void SetClip(const wstring& newClip);

	virtual COMPONENT GetType() const { return COMPONENT::ANIMATOR; }

private:
	void Ready_Layer();
	void Apply();

private:
	Animator* animator{};
	wstring clipName;
	uint32_t layer = 2;
	float targetWeight = 1.f;
	float playbackSpeed = 1.f;
};

NS_END