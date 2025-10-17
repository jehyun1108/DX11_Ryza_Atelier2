#pragma once

NS_BEGIN(Engine)

enum class VSyncMode : uint8_t { On, Off, Adaptive };

struct ENGINE_DLL DeviceOptions
{
	DXGI_FORMAT backbufferFormat  = DXGI_FORMAT_R8G8B8A8_UNORM; // SwapChain Format
	bool        useSRGBbackbuffer = true;                       // 최종 RTV만 sRGB
	bool        createDepthSRV    = true;
	_uint       backbufferCount   = 3; 
	VSyncMode   vsyncMode         = VSyncMode::On;
	bool        allowTearing      = true;       
	_uint       frameLimiterHz    = 0; // VSync Off 일때만 의미
};

NS_END