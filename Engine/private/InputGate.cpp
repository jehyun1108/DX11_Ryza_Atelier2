#include "Enginepch.h"

void InputGate::SetActiveEntity(EntityID entity)
{
	activeEntity = entity;
	elapsedSinceSwitch = 0.f;
}

void InputGate::AcquireLock(LockTag tag, int priority)
{
	ownerShip.lockTag = tag;
	ownerShip.priority = priority;
}

void InputGate::ReleaseLock(LockTag tag)
{
	if (ownerShip.lockTag == tag)
		ownerShip.lockTag = LockTag::None;
}

bool InputGate::Allow(InputChannel channel, EntityID targetEntity) const
{
	// 1. Lock/Focus 우선 (CutScene/Menu Lock 이면 Script만 허용)
	if (ownerShip.lockTag == LockTag::CutScene || ownerShip.lockTag == LockTag::MenuLock)
		return (channel == InputChannel::Script);

	// UI Focus면 Manual 차단 (전투 행동 선택 등)
	if (ownerShip.focus == FocusState::UI && channel == InputChannel::Manual)
		return false;

	// 2. Switching 직후 Manual 차단(Active 대상에 한정)
	if (channel == InputChannel::Manual && targetEntity == activeEntity)
	{
		if (elapsedSinceSwitch < config.blockManualTime)
			return false;
	}

	// 3. Routing Rule: Manual은 오직 Active 대상에게만 허용
	if (channel == InputChannel::Manual && targetEntity != activeEntity)
		return false;

	// 4. 나머지는 기본 허용(AI/Script)
	return true;
}