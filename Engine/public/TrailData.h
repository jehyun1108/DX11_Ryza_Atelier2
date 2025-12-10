#pragma once

NS_BEGIN(Engine)

enum class TrailShapeMode
{
	FollowPath,  // 실제 궤적
	ArcAnalytic  // 시간에 따라 계산된 원호
};
enum class TrailArcPlane
{
	Default, Horizontal, Vertical
};
struct TrailDesc   // 이 궤적이 어떤 성격인지?
{
	_float           lifeTime;
	_float           widthStart, widthEnd;
	_float4          colorStart, colorEnd;
	EffectCurveType  widthCurve, alphaCurve;
	_float           minSegDist;
	wstring          texKey;

	// ------ 호모 양 전용 파라미터 ========================
	TrailShapeMode  shapeMode         = TrailShapeMode::ArcAnalytic;
	TrailArcPlane   arcPlane          = TrailArcPlane::Default;

	float           arcRadius         = 200.f;
	float           arcStartDeg       = -90.f; 
	float           arcEndDeg         = 90.f;
	bool            arcUseownerCenter = true;
	_float3         arcCenterOffset   = { 0.f, 0.f, 0.f };
	_float3         arcRotDeg = { 0.f, 0.f, 0.f };

	// ------- 호 위에서 튀는 스파크 파티클 -----------------
	bool              sparkEnabled = false;
	float             sparkInterval = 0.02f;     // 몇 초마다 한 번
	int               sparkBurstCount = 4;       // 한 번에 몇 개
	ParticleSpawnData spark;                     // 스파크 모양/색/속도/텍스처

	bool   sheetEnabled = false;
	int    sheetCols = 1;
	int    sheetRows = 1;
	int    sheetStartFrame = 0;
	int    sheetEndFrame = 0;
	float  sheetFps = 0.f;

	SpriteSheetInfo sheet;
};
struct TrailPoint   // 어떤 시점에 어디를 지나갔는지
{
	_float3 pos;
	_float  age;
};
struct TrailInstance // Trail 한줄 전체의 상태
{
	vector<TrailPoint> points;
	_float             elapsed;
	TrailDesc          desc;
	EntityID           owner;

	float              sparkAccum = 0.f;
};
struct TrailRuntime
{
	TrailInstance inst;
	bool          alive = false;
};
NS_END