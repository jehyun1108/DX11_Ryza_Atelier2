#pragma once

NS_BEGIN(Engine)

enum class FaceState { OpenHold, Closing, CloseHold, Opening };

class ENGINE_DLL FaceController final : public Component
{
public:
	explicit FaceController(Obj* owner, 
		                    wstring openClip,                // BasePose 용 (재생 0x)
		                    wstring closeClip,               // 눈 닫힘 클립 (weight 로 제어)
		                    float openDur    = 2.5f,         // 평균 개안 유지 시간
		                    float openJitter = 1.2f,          // 개안 시간 변동폭
		                    float closeHold  = 0.12f,        // 감은 상태 유지
		                    float closeFade  = 0.08f,        // 눈감기 페이드시간
		                    float openFade   = 0.08f);       // 눈드는 페이드시간

public:
	virtual void Update(float dt) override;

	void SetSpeed(float scale) { speedScale = max(0.1f, scale); }
	float GetSpeed() const { return speedScale; }

	virtual COMPONENT GetType() const { return COMPONENT::ANIMATOR; }

private:
	float NextOpenHold() const;
	void Ready_Layer();
	void RebuildMaskFromClip();

private:
	Animator* animator{};
	uint32_t layer = 1;

	wstring clipOpen;   
	wstring clipClose;  

	float openDur      = 0.f;
	float openJitter   = 0.f;
	float holdCloseDur = 0.f;     // 빨리감기
	float fadeClose    = 0.f;     // 빨리뜨기
	float fadeOpen     = 0.f;   // 감은 상태 짧게

	float curFadeClose = 0.f;
	float curFadeOpen  = 0.f;
	float curOpenHold  = 0.f;

	FaceState state = FaceState::OpenHold;
	float timer = 0.f;

	float speedScale = 1.f;
};

NS_END