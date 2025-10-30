#pragma once

#include "InputRouting.h"
#include "SkyboxSystem.h"

NS_BEGIN(Engine)

// InputGate: "허용/차단" 정책 판단 (Lock/Focus/Switching/Active Routing)
class ENGINE_DLL InputGate
{
public:
	void     SetActiveEntity(EntityID entity);
	EntityID GetActiveEntity() const { return activeEntity; }

	void SetContext(InputContext newContext)          { config.context = newContext; }
	InputContext GetContext() const { return config.context; }

	void SetOwnerShip(const InputOwnerShip& newOwner) { ownerShip = newOwner; }
	void SetFocus(FocusState newFocus)                { ownerShip.focus = newFocus; }
	void SetBlockManualTime(float time)               { config.blockManualTime = time; }
	void Tick(float dt)                               { elapsedSinceSwitch += dt; }

	void AcquireLock(LockTag tag, int priority);
	void ReleaseLock(LockTag tag);

	// 핵심: 지금 프레임에 이 채널이 이 타깃에게 써도 되는지?
	bool Allow(InputChannel channel, EntityID targetEntity) const;

private:
	InputRoutingConfig config{};
	InputOwnerShip     ownerShip{};
	EntityID           activeEntity = invalidEntity;
	float              elapsedSinceSwitch = 999.f;
};

NS_END