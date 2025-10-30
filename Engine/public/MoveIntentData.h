#pragma once

NS_BEGIN(Engine)

struct ENGINE_DLL MoveIntent
{
	_float2 moveDir{};
	float   turnInput = 0.f;
	bool    isRunning = false;
};

NS_END