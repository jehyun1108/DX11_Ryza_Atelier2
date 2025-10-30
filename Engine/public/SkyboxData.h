#pragma once

NS_BEGIN(Engine)

struct SkyboxState
{
	Handle tf{};
	bool enabled = false;

	vector<SkySubmesh> submeshes{};
	SkyTextureType texType = SkyTextureType::Equirect2D;

	// 카메라 연동 + 공간 변환
	bool  attachToCam  = true; // 위치를 카메라에 스냅 (pitch/roll 영향x)
	float uniformScale = 1000.f;

	// 회전 Yaw만 사용 (수평선 왜곡 방지)
	float baseYawRad = 0.f; // 시작 각
	float rotSpeed   = 0.f;
	float phaseRad   = 0.f; // 누적 각
};

struct SkyboxCrossFade
{
	bool active      = false;
	_uint fromId     = 0;
	_uint toId       = 0;
	float progress01 = 0.f;
	float dur        = 1.f;
};

struct SkyRule
{
	SkyQueue queue = SkyQueue::Opaque;
	SkyCull  cull  = SkyCull::Back;
	bool transparent   = false;
	bool premultiplied = false;
};

NS_END