#include "Enginepch.h"

// Dispatch 로직개선 여지 있음
void BattleEventBus::DispatchAll()
{
	if (eventQueue.empty() || listenersById.empty()) return;

	for (const BattleEvent& event: eventQueue)
	{
		for (const auto& pair : listenersById)
		{
			const ListenerEntry& entry = pair.second;

			if (!entry.typeFilter.has_value() || entry.typeFilter.value() == event.eventType)
			{
				entry.listener(event);
			}
		}
	}
}

BattleEventListenerId BattleEventBus::Subscribe(optional<BattleBusEventType> typeFilter, BattleEventListener listener)
{
	const BattleEventListenerId newId = nextListenerId++;
	listenersById.emplace(newId, ListenerEntry{ typeFilter, move(listener) });
	return newId;
}

bool BattleEventBus::Unsubscribe(BattleEventListenerId listenerId)
{
	auto it = listenersById.find(listenerId);
	if (it == listenersById.end())
		return false;

	listenersById.erase(it);
	return true;
}
