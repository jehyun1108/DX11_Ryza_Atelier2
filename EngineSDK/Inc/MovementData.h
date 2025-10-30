#pragma once

NS_BEGIN(Engine)

struct ENGINE_DLL MoveIntent // 움직임 의도
{
	_float2 moveXY{};        // 캐릭터 상하좌우 움직임방향
	bool    jump = false;    // 이번 프레임 jump 요청의도
	bool    jumpHeld = false;
	float   yawDeg = 0.f;    // 바라볼 목표 Yaw 도, 안쓰면 0
};

struct ENGINE_DLL MovementData
{
	Handle tfHandle;

	// 튜닝: 값만 바꿔도 움직임 느낌조절
	float maxSpeed   = 5.f;
	float accelSpeed = 12.f;
	float decelSpeed = 14.f;

	float jumpSpeed  = 4.5f;
	float gravityY   = -9.8f;
	float airControl = 0.35f;  // 공중 제어 비율 (0~1)

	float gravityRiseScale = 0.6f;   // 상승 중 적용(약하게 ↑ 오래 떠있음)
	float gravityFallScale = 2.2f;   // 하강 중 적용(강하게 ↓ 빠른 낙하)
	float jumpCutFactor    = 0.60f;  // 점프 중 버튼 떼면 y속도 *= 이 값
	float maxFallSpeed     = 85.f;   // 낙하 최대속도 클램프(|vY| 상한)

	// 런타임 상태
	_float3 velocityWorld{};
	bool    grounded = false;
	float   sinceGround = numeric_limits<float>::infinity(); // 마지막 접지 후 경과 초
	_float3 groundNormal = { 0.f, 1.f, 0.f }; // 지면 Normal

	// 바라볼지
	bool useYaw = true;

	// 현재 프레임 의도
	MoveIntent intent{};
};

NS_END