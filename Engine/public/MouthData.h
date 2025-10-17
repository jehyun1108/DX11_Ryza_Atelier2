#pragma once

NS_BEGIN(Engine)

struct ENGINE_DLL MouthData
{
	Handle animator;
	_uint layer = 2;
	wstring clip;
	float weight = 1.f;
	float speed = 1.f;
};

NS_END