#pragma once

NS_BEGIN(Engine)

struct CameraProxy
{
	_float4x4 view;
	_float4x4 proj;
	_float4x4 invView;
	_float4x4 invProj;
	_float4x4 viewProj;
	_float4x4 invViewProj;

	_float4   camForward;
	_float4   camPos;
	float     zNear;
	float     zFar;
	float     fovY;
	float     aspect;
};

struct LightProxy
{
	int       type = ENUM(LIGHT::DIRECTIONAL);
	float     range = 100.f;
	float     spotAngle = XM_PI / 4.f;
	float     padding{};

	//_float4   ambient = { 0.25f, 0.25f, 0.25f, 1.f };
	_float4   ambient = { 0.25f, 0.25f, 0.3f, 1.f };
	_float4   diffuse = { 1.2f, 1.2f, 1.2f, 1.f };
	_float4   specular = { 1.f, 1.f, 1.f, 1.f };
	_float4   lightPos;
	_float4   lightDir = { 0.5f, -1.0f, 0.3f, 0.0f };
};

struct ObjCB
{
	_float4x4 world;
	_float4   color{};
	_float4x4 invWorld;
	_float4   vpSize{};
	_uint     boneBase;
	_uint     boneCount;
	_float2   padding;
};

struct SkyCB
{
	float   theta;
	float   opacity;
	int     isPremultiplied; // 0: Straight, 1: premultiplied
	float   padding;
};

struct UICB
{
	float   screenW;
	float   screenH;
	float   invScreenW;
	float   invScreenH;

	_float2 minimapCenter;
	float   minimapRadius;
	float   minimapMaskEnable;
};

struct MaterialCB
{
	float   AO            = 1.f;           
	float   Shininess     = 32.f;  
	float   SpecIntensity = 2.f; 
	float   padding0;

	_float3 SpecColor     = { 0.2f, 0.2f, 0.2f }; 
	float   Emissive      = {};     
};

struct PostCB
{
	float exposure      = 1.6f; 
	float aoStrength    = 0.7f;
	float bloomStrength = 0.08f;
	float whitePoint    = 6.f;
};

struct DebugCB
{
	int   mode;
	float scale;
	float pad[2];
};

struct DistortionCB
{
	_float2 centerUV;    // 충격파 중심 (0~1)
	float   radius;      // 현재 파동 반경
	float   thickness;   // 파동의 두께

	float   strength;    // 기본 왜곡 세기 (UV 오프셋 크기)
	float   time;        // 시작 후 경과 시간
	float   duration;    // 전체 연출 시간
	float   paddingDist; // 정렬용
};

struct TrailCB
{
	_uint trailCols;
	_uint trailRows;
	_uint trailStartFrame;
	_uint trailEndFrame;
	float trailFps;
	float trailTime;
	_uint trailSheetEnabled;
	float pad;
};

NS_END