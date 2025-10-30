#pragma once

#include "MoveIntentData.h"

NS_BEGIN(Engine)

// PlayerController 가 입력 해석(Mapping, Edge검출, Toggle, DeadZone) 을 맡아 매프레임 intent를 채워넣는 방식
class ENGINE_DLL MoveIntentSystem : public EntitySystem<MoveIntent> // 저장소 역할
{
public:
	explicit MoveIntentSystem(SystemRegistry& registry) : EntitySystem(registry) {}

	Handle Create(EntityID owner);

	// 프레임 시작에 초기화 (일회성 버퍼)
	void Clear(EntityID owner);
	void SetIntent(EntityID owner, const MoveIntent& in);
};

NS_END