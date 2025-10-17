#pragma once

NS_BEGIN(Engine)

enum class FaceState { OpenHold, Closing, CloseHold, Opening };

struct ENGINE_DLL FaceData
{
	Handle animator;
	_uint layer = 1;

	wstring clipOpen;
	wstring clipClose;

	float openDur    = 2.5f;
	float openJitter = 1.2f;
	float holdClose  = 0.12f;
	float fadeClose  = 0.08f;
	float fadeOpen   = 0.08f;

	float curOpenHold = 0.f;
	float curFadeClose = 0.f;
	float curFadeOpen = 0.f;
	FaceState state = FaceState::OpenHold;
	float timer = 0.f;
	float speedScale = 1.f;
};

NS_END