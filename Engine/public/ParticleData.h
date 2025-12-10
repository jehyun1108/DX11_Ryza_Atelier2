#pragma once

NS_BEGIN(Engine)
struct EffectEmitterDesc;

using EffectHandle = _uint;
enum class ParticleVisualMode
{
	Billboard, SpriteSheet
};
struct SpriteSheetInfo
{
	bool enabled = false;
	bool animate = true;

	int  cols = 1;
	int  rows = 1;

	int   startFrame = 0;
	int   endFrame   = 0;

	float fps  = 0.f;
	bool  loop = true;
};
struct ParticleSpawnData // (정적 설계도) 파티클이 태어날 때 어떻게 초기화할지 정의하는 레시피 (새로운 파티클을 생성할때만 쓰임)
{
	float spawnRate;

	float lifeMin, lifeMax;
	float speedMin, speedMax;

	_float3 baseDir;
	float   spreadAng;

	float   startSize, endSize;
	_float4 startColor, endColor;

	EffectCurveType sizeCurve  = EffectCurveType::Linear;
	EffectCurveType alphaCurve = EffectCurveType::EaseOut;
	EffectCurveType colorCurve = EffectCurveType::Linear;
	EffectCurveType rateCurve  = EffectCurveType::Linear;

	EffectPresetType presetType = EffectPresetType::None;

	// --- 회전 / 방향 제어용 추가 필드 --------------------------
	bool   dirLocal       = false;  // true면 baseDir을 로컬 기준으로 해석
	float  rotSpeedMin    = 0.f;    // 스핀 속도 범위 (rad/sec)
	float  rotSpeedMax    = 0.f;
	bool   randomStartRot = false;  // true면 시작 각도 랜덤 (0~2π)

	wstring texKey;
	SpriteSheetInfo sheet;

	ParticleVisualMode visualMode = ParticleVisualMode::Billboard;

	float posRadiusMin = 0.f; // 0 이면 한 점에서 시작
	float posRadiusMax = 0.f; // >0 이면 구 내부에서 랜덤
	bool   velFromPos = false;
};
struct Particle // 입자 한 알의 현재 상태
{
	const ParticleSpawnData* data{};
	EffectHandle owner;

	_float3 pos, velocity;
	float   aliveTime, lifeTime;

	float   size{};
	_float4 color{};

	float   rotRad, rotSpeed;
	wstring texKey;

	int     sheetFrame = 0;
};
struct ParticleSpawnerInstance // 지금 씬에서 어떻게 돌아가고 있는지 런타임 인스턴스 데이터 
{
	const ParticleSpawnData* data{};

	_float3 worldPos{};
	float   elapsed{};
	float   spawnAccum{};

	_float3 basisRight   = _float3(1.f, 0.f, 0.f);
	_float3 basisUp      = _float3(0.f, 1.f, 0.f);
	_float3 basisForward = _float3(0.f, 0.f, 1.f);
	bool    useBasis     = false;

	EffectHandle owner = 0;
};

NS_END