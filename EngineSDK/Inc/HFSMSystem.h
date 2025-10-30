#pragma once

NS_BEGIN(Engine)

// 2단 Mode, Shallow History, ESC Bubbling 처리
class ENGINE_DLL HFSMSystem : public EntitySystem<ModeHFSMData>, public IGuiRenderable
{
public:
	explicit HFSMSystem(SystemRegistry& registry, const ActionRegistry* actionRegistry)
		: EntitySystem(registry), actionRegistry(actionRegistry) {}

	Handle Create(EntityID owner, ModeLeafState init = ModeLeafState::Field);
	// 1. Bubbling 처리 -> 2. 전이 0~1 회 적용
	void Update(float dt);
	void RenderGui(EntityID id) override;

private:
	// ESC/전역메뉴/전투요청을 Mode Level 에서 처리 (Bubbling 상위 관리자)
	void HandleAtModeLevel(EntityID owner, ModeHFSMData& mode, StateEvent& event);
	void RequestTransition(ModeHFSMData& mode, ModeLeafState target);
	void ApplyTransition(EntityID owner, ModeHFSMData& mode, ModeLeafState target);
	void InvokeEnter(EntityID owner, ModeLeafState fromState, ModeLeafState toState);
	const char* ToCString(ModeLeafState state);

private:
	const ActionRegistry* actionRegistry{};
};

NS_END