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
	// Context에 따라 허용할 입력 종류(매핑)는 컨트롤러가 처리하고, Router는 "채널 기록 허용/차단만 담당"
	InputContext context = InputContext::Field;
	// Switching 직후 Manual 잠시 차단
	float blockManualTime = 0.2f;
};

// Controller 가 제출하는 단위
struct IntentWrite
{
	EntityID     target{};
	InputChannel channel{};
	MoveIntent   intent{};
};

struct IntentSnapShot
{
	// map 자체를 복사하지 않도록 포인터로 뷰를 제공
	// Collector 가 Clear를 호출하기전까지 (같은 프레임내)
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