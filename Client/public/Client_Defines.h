#pragma once

// WINX/ WINY : (720/480) , (1280/720), (1920/1080)
// DefaultX/DefaultY : 2300 / 100 ,       

namespace Client
{
	static constexpr int WinX = 1920;
	static constexpr int WinY = 1080;

	static constexpr int DefaultX = 2300;
	static constexpr int DefaultY = 100;

	enum class LEVEL {STATIC, LOADING, LOGO, GAMEPLAY, END};
}

extern HINSTANCE g_hInst;
extern HWND g_hWnd;

using namespace Client;