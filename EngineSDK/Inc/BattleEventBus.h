#pragma once

#include "BattleEventBusData.h"

NS_BEGIN(Engine)

// Publish: 여러 시스템(Session/Timeline/Execution/Resolve)이 사건을 게시
// Queue: 프레임 동안 사건을 큐에 모음
// DispatchAll: Listener에게 일괄 Dispatch
// Subscribe/Unsubscribe: 이벤트 타입별로 콜백 구독/해지
class ENGINE_DLL BattleEventBus
{
public:
	void Publish(const BattleEvent& eventData)   { eventQueue.push_back(eventData); }

	const vector<BattleEvent>& PeekQueue() const { return eventQueue; }
	void ClearQueue()                            { eventQueue.clear(); }

	void DispatchAll();

	BattleEventListenerId Subscribe(optional<BattleBusEventType> typeFilter, BattleEventListener listener);
	bool Unsubscribe(BattleEventListenerId listenerId);
	void ReserveQueue(size_t n) { eventQueue.reserve(n); }

private:
	struct ListenerEntry
	{
		optional<BattleBusEventType> typeFilter;
		BattleEventListener          listener;
	};

	vector<BattleEvent>                                 eventQueue;
	unordered_map<BattleEventListenerId, ListenerEntry> listenersById;
	BattleEventListenerId                               nextListenerId = 1;
};

NS_END