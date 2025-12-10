#include "Enginepch.h"

void BattleEventBus::DispatchAll()
{
    if (listenersById.empty())
    {
        eventQueue.clear();
        return;
    }

    vector<BattleEvent> toDispatch;
    toDispatch.swap(eventQueue); 

    for (const BattleEvent& event : toDispatch)
    {
        for (const auto& pair : listenersById)
        {
            const ListenerEntry& entry = pair.second;
            if (!entry.typeFilter.has_value() || entry.typeFilter.value() == event.eventType)
                entry.listener(event);
        }
    }
}

_uint BattleEventBus::Subscribe(optional<BattleBusEventType> typeFilter, BattleEventListener listener)
{
	const _uint newId = nextListenerId++;
	listenersById.emplace(newId, ListenerEntry{ typeFilter, move(listener) });
	return newId;
}

bool BattleEventBus::Unsubscribe(_uint listenerId)
{
	auto it = listenersById.find(listenerId);
	if (it == listenersById.end()) return false;
	listenersById.erase(it);
	return true;
}