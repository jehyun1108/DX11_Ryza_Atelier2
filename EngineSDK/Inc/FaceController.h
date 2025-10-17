#pragma once

NS_BEGIN(Engine)

enum class FaceState { OpenHold, Closing, CloseHold, Opening };

class ENGINE_DLL FaceController final : public Component
{
public:
	explicit FaceController(Obj* owner, 
		                    wstring openClip,                // BasePose �� (��� 0x)
		                    wstring closeClip,               // �� ���� Ŭ�� (weight �� ����)
		                    float openDur    = 2.5f,         // ��� ���� ���� �ð�
		                    float openJitter = 1.2f,          // ���� �ð� ������
		                    float closeHold  = 0.12f,        // ���� ���� ����
		                    float closeFade  = 0.08f,        // ������ ���̵�ð�
		                    float openFade   = 0.08f);       // ����� ���̵�ð�

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
	float holdCloseDur = 0.f;     // ��������
	float fadeClose    = 0.f;     // �����߱�
	float fadeOpen     = 0.f;   // ���� ���� ª��

	float curFadeClose = 0.f;
	float curFadeOpen  = 0.f;
	float curOpenHold  = 0.f;

	FaceState state = FaceState::OpenHold;
	float timer = 0.f;

	float speedScale = 1.f;
};

NS_END