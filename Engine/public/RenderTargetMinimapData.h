#pragma once

NS_BEGIN(Engine)

struct MinimapRTSpec
{
	_uint width  = 224;
	_uint height = 225;

	DXGI_FORMAT colorFmt    = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT depthTexFmt = DXGI_FORMAT_R24G8_TYPELESS;
	DXGI_FORMAT depthDsvFmt = DXGI_FORMAT_D24_UNORM_S8_UINT;
	DXGI_FORMAT depthSrvFmt = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
};

NS_END