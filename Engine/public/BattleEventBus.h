#pragma once

#include "BattleEventBusData.h"

NS_BEGIN(Engine)

class ENGINE_DLL BattleEventBus : public ISystem
{
public:
	explicit BattleEventBus(SystemRegistry& registry) : registry(registry) {}

	void Publish(const BattleEvent& eventData)   { eventQueue.push_back(eventData); }

	const vector<BattleEvent>& PeekQueue() const { return eventQueue; }
	void ClearQueue()                            { eventQueue.clear(); }
	void DispatchAll();

	_uint Subscribe(optional<BattleBusEventType> typeFilter, BattleEventListener listener);
	bool Unsubscribe(_uint listenerId);
	void ReserveQueue(size_t n) { eventQueue.reserve(n); }

private:
	SystemRegistry& registry;
	struct ListenerEntry
	{
		optional<BattleBusEventType> typeFilter;
		BattleEventListener          listener;
	};

	vector<BattleEvent>                                 eventQueue;
	unordered_map<_uint, ListenerEntry> listenersById;
	_uint                               nextListenerId = 1;
};

NS_END