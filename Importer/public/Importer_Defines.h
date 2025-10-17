#pragma once

// WinX: 1920, 2560, 1440
// WinY: 1080, 1440, 720

namespace Importer
{
	static constexpr int WinX = 2560;
	static constexpr int WinY = 1440;
	static constexpr int DefaultX = 1920;
	static constexpr int DefaultY = 0;

	//static constexpr int WinX = 1920;
	//static constexpr int WinY = 1080;

	//static constexpr int DefaultX = 0;
	//static constexpr int DefaultY = 0;


	enum class LEVEL { IMPORTER, END };
}

extern HINSTANCE g_hInst;
extern HWND g_hWnd;