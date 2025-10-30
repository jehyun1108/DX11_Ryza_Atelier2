#include "Enginepch.h"

void InputService::EndFrameAndApply(SystemRegistry& registry)
{
	const IntentSnapShot snapShot = intentCollector.GetSnapShot();
	IntentMerger::MergeAndApply(registry, snapShot);
	intentCollector.Clear();
}

bool InputService::KeyDown(KEY key) const
{
	const EntityID activeEntity = inputGate.GetActiveEntity();
	if (!PassPolicy(InputChannel::Manual, activeEntity)) return false;
	return GAME.KeyDown(key);
}

bool InputService::KeyPressing(KEY key) const
{
	const EntityID activeEntity = inputGate.GetActiveEntity();
	if (!PassPolicy(InputChannel::Manual, activeEntity)) return false;
	return GAME.KeyPressing(key);
}

bool InputService::KeyReleased(KEY key) const
{
	const EntityID activeEntity = inputGate.GetActiveEntity();
	if (!PassPolicy(InputChannel::Manual, activeEntity)) return false;
	return GAME.KeyRelease(key);
}

bool InputService::KeyDownAllowed(KEY key, InputChannel channel, EntityID target) const
{
	if (!PassPolicy(channel, target)) return false;
	return GAME.KeyDown(key);
}

bool InputService::KeyPressingAllowed(KEY key, InputChannel channel, EntityID target) const
{
	if (!PassPolicy(channel, target)) return false;
	return GAME.KeyPressing(key);
}

bool InputService::KeyReleasedAllowed(KEY key, InputChannel channel, EntityID target) const
{
	if (!PassPolicy(channel, target)) return false;
	return GAME.KeyRelease(key);
}

bool InputService::KeyDownGlobal(KEY key) const
{
	return GAME.KeyDown(key);
}

bool InputService::KeyPressingGlobal(KEY key) const
{
	return GAME.KeyPressing(key);
}

bool InputService::KeyReleasedGlobal(KEY key) const
{
	return GAME.KeyRelease(key);
}

void InputService::Submit(const IntentWrite& write)
{
	if (inputGate.Allow(write.channel, write.target))
		intentCollector.Submit(write);
}

void InputService::PushJumpEdge(EntityID target, InputChannel channel)
{
	if (inputGate.Allow(channel, target))
		jumpEdgeByEntity[target] = true;
}

bool InputService::ConsumeJumpEdge(EntityID entity)
{
	auto it = jumpEdgeByEntity.find(entity);
	const bool wasEdge = (it != jumpEdgeByEntity.end() && it->second);
	if (wasEdge) it->second = false;
	return wasEdge;
}

void InputService::PushAttackEdge(EntityID target, InputChannel channel)
{
	if (inputGate.Allow(channel, target))
		attackEdgeByEntity[target] = true;
}

bool InputService::ConsumeAttackEdge(EntityID entity)
{
	auto it = attackEdgeByEntity.find(entity);
	const bool wasEdge = (it != attackEdgeByEntity.end() && it->second);
	if (wasEdge) it->second = false;
	return wasEdge;
}