#pragma once

NS_BEGIN(Engine)

enum class InputChannel { AI, Manual, Script };
enum class InputContext { Field, Battle, Menu };
enum class FocusState   { None, UI };
enum class LockTag      { None, Casting, CutScene, MenuLock };

// Ownership/Focus/Lock + Routing Policy
struct InputOwnerShip
{
	// "현재 최상위 소유자" 의 힘(우선순위). 필요할때 Allow() 로직에 활용
	int        priority = 0;
	LockTag    lockTag  = LockTag::None;    // 현재 Lock 사유
	FocusState focus    = FocusState::None; // UI가 Focus를 잡았는지
};

struct InputRoutingConfig
{
	InputContext context = InputContext::Field;
	float blockManualTime = 0.2f;
};

struct IntentWrite
{
	EntityID     target{};
	InputChannel channel{};
	MoveIntent   intent{};
};

struct IntentSnapShot
{
	const unordered_map<EntityID, IntentWrite>* script = {};
	const unordered_map<EntityID, IntentWrite>* manual = {};
	const unordered_map<EntityID, IntentWrite>* ai     = {};
};

struct UiFocusPolicy
{
	bool wantKeyboard = false;
	bool wantMouse    = false;
	bool isMenuOpen   = false;
};

NS_END