#pragma once

NS_BEGIN(Engine)

enum class VSyncMode : uint8_t { On, Off, Adaptive };

struct ENGINE_DLL DeviceOptions
{
	DXGI_FORMAT backbufferFormat  = DXGI_FORMAT_R8G8B8A8_UNORM; // SwapChain Format
	bool        useSRGBbackbuffer = true;                       // ���� RTV�� sRGB
	bool        createDepthSRV    = true;
	_uint       backbufferCount   = 3; 
	VSyncMode   vsyncMode         = VSyncMode::On;
	bool        allowTearing      = true;       
	_uint       frameLimiterHz    = 0; // VSync Off �϶��� �ǹ�
};

NS_END