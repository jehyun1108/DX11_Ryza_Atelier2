#pragma once

NS_BEGIN(Engine)

enum class ModeLeafState{ Field, Battle, Menu };
// 전역성 소수 Event : ESC, 전투 On/Off, Menu Open/Close
enum class EventKey
{
	EscPressed,
	EnterBattle, 
	ExitBattle,
	OpenMenu,
	CloseMenu
};

struct StateEvent
{
	EventKey eventKey{};
	bool     isHandled = false;
};

struct EventInboxData
{
	vector<StateEvent> pendingEvents;
};

struct ModeHFSMData
{
	ModeLeafState activeLeaf = ModeLeafState::Field;
	optional<ModeLeafState> shallowHistoryPrevLeaf{};
	optional<ModeLeafState> pendingTransitionTarget{};
};

// 얇은 ActionRegistry: 문자열->콜백(Registry, EntityID, 이전/다음 상태)
using ActionFunc = function<void(SystemRegistry&, EntityID, ModeLeafState /*from*/, ModeLeafState /*to*/)>;

struct ActionRegistry
{
	unordered_map<string, ActionFunc> actions;

	void Invoke(const string& actionKey, SystemRegistry& registry, EntityID owner, ModeLeafState fromState, ModeLeafState toState) const
	{
		auto it = actions.find(actionKey);
		if (it != actions.end() && it->second)
			it->second(registry, owner, fromState, toState);
	}
};

NS_END