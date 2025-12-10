#pragma once

NS_BEGIN(Engine)

struct GroundingComponent
{
	float castUp     = 30.f;   // 레이 시작을 y위로 올리는 높이
	float castDown   = 30.f;    // 아래로 쓸 최대 거리
	bool  enabled    = true;

};

NS_END