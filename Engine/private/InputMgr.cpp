#include "Enginepch.h"

HRESULT InputMgr::Init()
{
	keyStates.resize(KEY_COUNT);

	// ---- RawInput ----
	RAWINPUTDEVICE rid{};
	rid.usUsagePage = 0x01;
	rid.usUsage     = 0x02;
	rid.dwFlags     = RIDEV_INPUTSINK;
	rid.hwndTarget  = g_hWnd;

	if (RegisterRawInputDevices(&rid, 1, sizeof(RAWINPUTDEVICE)) == FALSE)
		return E_FAIL;

	return S_OK;
}

void InputMgr::BeginFrame()
{
	POINT pt{};
	GetCursorPos(&pt);
	ScreenToClient(g_hWnd, &pt);
	mousePos = { (float)pt.x, (float)pt.y };
}

void InputMgr::EndFrame()
{
	for (auto& key : keyStates)
	{
		if (key.state == KEY_STATE::DOWN)
			key.state = KEY_STATE::PRESSING;
		else if (key.state == KEY_STATE::RELEASE)
			key.state = KEY_STATE::NONE;
	}
	mouseDelta = {};
}

void InputMgr::ProcessWinMsg(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INPUT:
	{
		if (!isActive) break;
		_uint size = 0;
		GetRawInputData((HRAWINPUT)lParam, RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER));

		static vector<BYTE> buffer;
		buffer.resize(size);

		if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, buffer.data(), &size, sizeof(RAWINPUTHEADER)) == size)
		{
			RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(buffer.data());
			if (raw->header.dwType == RIM_TYPEMOUSE) {
				mouseDelta.x += (float)raw->data.mouse.lLastX;
				mouseDelta.y += (float)raw->data.mouse.lLastY;

				const USHORT buttonFlags = raw->data.mouse.usButtonFlags;

				if (buttonFlags & RI_MOUSE_LEFT_BUTTON_DOWN)
					keyStates[(size_t)KEY::LBUTTON].state = KEY_STATE::DOWN;
				if (buttonFlags & RI_MOUSE_LEFT_BUTTON_UP)
					keyStates[(size_t)KEY::LBUTTON].state = KEY_STATE::RELEASE;

				if (buttonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN)
					keyStates[(size_t)KEY::RBUTTON].state = KEY_STATE::DOWN;
				if (buttonFlags & RI_MOUSE_RIGHT_BUTTON_UP)
					keyStates[(size_t)KEY::RBUTTON].state = KEY_STATE::RELEASE;
			}
		}
	}break;

	// ------------- Keyboard ------------------
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	{
		const KEY key = Win32::MapVirtualToKey(wParam, lParam);
		if (key == KEY::END) break;

		const bool wasDown = ((lParam & Win32::PREV_KEY_STATE_MASK) != 0);
		if (!wasDown) 
			keyStates[(size_t)key].state = KEY_STATE::DOWN;
	} break;

	case WM_KEYUP:
	case WM_SYSKEYUP:
	{
		const KEY key = Win32::MapVirtualToKey(wParam, lParam);
		if (key == KEY::END) break;

		keyStates[(size_t)key].state = KEY_STATE::RELEASE;
	} break;

    // ----------- Active & Focus -----------------
	case WM_ACTIVATE:
	{
		isActive = (LOWORD(wParam) != WA_INACTIVE);
		if (!isActive)
		{
			for (auto& key : keyStates) 
				key.state = KEY_STATE::NONE;
			mouseDelta = {};
		}
	} break;

	case WM_KILLFOCUS:
	{
		isActive = false;
		for (auto& key : keyStates) 
			key.state = KEY_STATE::NONE;
		mouseDelta = {};
	} break;

	default: break;
	}
}

unique_ptr<InputMgr> InputMgr::Create()
{
	auto instance = make_unique<InputMgr>();

	if (FAILED(instance->Init()))
		return nullptr;

	return instance;
}