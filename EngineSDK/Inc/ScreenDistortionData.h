#pragma once

NS_BEGIN(Engine)

struct ScreenDistortionState
{
	bool active = false;
	DistortionCB cb{};
};

NS_END