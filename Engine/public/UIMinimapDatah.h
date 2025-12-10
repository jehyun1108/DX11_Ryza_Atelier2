#pragma once

NS_BEGIN(Engine)

enum class MinimapMode { None, Field, Battle };

struct MinimapCamDesc
{
	CameraProxy cam;
	float worldRadius = 1000.f;
};

NS_END