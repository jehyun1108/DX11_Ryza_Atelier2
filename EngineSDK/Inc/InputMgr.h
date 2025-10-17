#pragma once

#include "InputUtil.h"

NS_BEGIN(Engine)

class ENGINE_DLL InputMgr final
{
public:
	static unique_ptr<InputMgr> Create();

	HRESULT Init();

	void BeginFrame();
	void EndFrame();

	void ProcessWinMsg(UINT msg, WPARAM wParam, LPARAM lParam);

	KEY_STATE GetKeyState(KEY key) const { return keyStates[static_cast<size_t>(key)].state; }
	bool      KeyPressing(KEY key) const { return GetKeyState(key) == KEY_STATE::PRESSING; }
	bool      KeyDown(KEY key)     const { return GetKeyState(key) == KEY_STATE::DOWN; }
	bool      KeyRelease(KEY key)  const { return GetKeyState(key) == KEY_STATE::RELEASE; }

	const _float2& GetMousePos()   const { return mousePos; }
	const _float2& GetMouseDelta() const { return mouseDelta; } 

private:
	vector<KeyInfo> keyStates;

	_float2 mousePos{};
	_float2 mouseDelta{}; 

	bool isActive = true;
};

NS_END