#pragma once

NS_BEGIN(Engine)

enum class EmitterShapePreset
{
	Single = 0,
	RadialBurst = 1,
};
enum class EffectSpaceMode // 좌표계 / 붙는 방식 
{
	Local, 
	World  
};
enum class EffectAttachType 
{
	None,     
	Transform  
};
enum class EffectRenderLayer
{
	World,      
	UI,          
	PostProcess  
};
enum class EffectEventType //(추후 PlayAudio, CameraShake 추가)
{
	SpawnEmitter, // 특정 Emitter 시작
	StopEmitter,  // 특정 Emitter 중지
};
enum class EffectEmitterKind 
{
	Particle, Trail
};

struct EffectEmitterDesc // 파티클 발사기 설계도
{
	wstring           name;
	EffectEmitterKind kind = EffectEmitterKind::Particle;

	_float3           localOffset{}; 
	EffectSpaceMode   spaceMode = EffectSpaceMode::World;

	bool              burst      = false; // 이 Emitter가 계속 파티클을 쏘는 continuous 타입인지, 아니면 딱 한번에 여러 개 터뜨리는 burst 타입인지
	int               burstCount = 0;     // burst 가 true 일때만 의미
	float             delay      = 0.f;
	float             duration   = -1.f; 

	ParticleSpawnData particle;    // 입자 한 알을 어떻게 초기화할지
	TrailDesc         trail;

	bool              useColliderObbTip = false;
};
struct EffectEventDesc // 이 타이밍에 무슨 일을 할지 종류를 표시하는 값
{
	float           time = 0.f;                           // 이펙트 시작 후 몇 초 시점에 실행할 건지
	EffectEventType type = EffectEventType::SpawnEmitter; // 그 시점에 어떤 종류의 행동을 할 건지
	int             emitterIdx = -1;                      // 어떤 EmitterDesc를 대상으로 할 건지 // 추후 Sound, CamerShake 등 추가
};
struct EffectArchetype // 이펙트 종류의 설계도 (프리팹)
{
	wstring                   key;
	float                     duration = 1.f;
	EffectRenderLayer         layer = EffectRenderLayer::World;

	vector<EffectEmitterDesc> emitters;
	vector<EffectEventDesc>   events;
};
// 4. 런타임 Emitter 상태
struct EffectEmitterRuntime 
{                         
	int    emitterIdx = -1;
	Handle spawner{};
	Handle trail{};

	bool   active     = false;			
	bool   burstFired = false;

	// ArcAnalytic
	bool    arcBasisInit = false;
	_float3 arcCenter{};
	_float3 arcRight{};
	_float3 arcForward{};

	// Spark on trail
	float   sparkAccum = 0.f; 
};
struct EffectInstance // 지금 씬에서 실제로 실행 중인 이펙트 한 개의 전체 상태
{
	EffectHandle                 handle = 0u;
	const EffectArchetype*       archetype{};

	float                        elapsed  = 0.f;
	bool                         finished = false;

	float                        durationOverride = 0.f;
	float                        startDelay = 0.f;

	EffectAttachType             attachType = EffectAttachType::None;
	Handle                       followTf{};
	_float3                      worldPos{};
	_float3                      localOffset{};

	EntityID                     owner = 0u;
	EntityID                     attachOwner{}; 

	vector<EffectEmitterRuntime> emitters;
	size_t                       nextEventIdx = 0;
};

NS_END