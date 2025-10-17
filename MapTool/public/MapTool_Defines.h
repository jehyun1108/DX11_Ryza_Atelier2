#pragma once

// WinX: 1920, 2560, 1440
// WinY: 1080, 1440, 720

namespace MapTool
{
	static constexpr int WinX = 1440;
	static constexpr int WinY = 720;

	static constexpr int DefaultX = 100;
	static constexpr int DefaultY = 0;

	enum class LEVEL { MAPTOOL, END };
}

extern HINSTANCE g_hInst;
extern HWND g_hWnd;