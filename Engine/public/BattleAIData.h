#pragma once

NS_BEGIN(Engine)

struct AIConfig
{
	float eval_hz = 10.f; // ÆÇ´Ü ºóµµ
};

struct AIBlackboard
{
	double next_eval_sec = 0.0;
};

NS_END