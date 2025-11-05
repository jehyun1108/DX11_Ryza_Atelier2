#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL InputService : public ISystem
{
public:
	explicit InputService(SystemRegistry& registry) : registry(registry) {}
	void     OnBoot() override;

	void BeginFrame(float dt) { inputGate.Tick(dt); }
	void EndFrameAndApply(SystemRegistry& registry);
	// ------------------------------------------------------------------------
	bool KeyDown(KEY key) const;
	bool KeyPressing(KEY key) const;
	bool KeyReleased(KEY key) const;

	const _float2& GetMouseDelta() const { return input->GetMouseDelta(); }
	const _float2& GetMousePos()   const { return input->GetMousePos(); }

	bool KeyDownAllowed(KEY key, InputChannel channel, EntityID target) const;
	bool KeyPressingAllowed(KEY key, InputChannel channel, EntityID target) const;
	bool KeyReleasedAllowed(KEY key, InputChannel channel, EntityID target) const;

	bool KeyDownGlobal(KEY key)     const { return input->KeyDown(key); }
	bool KeyPressingGlobal(KEY key) const { return input->KeyPressing(key); }
	bool KeyReleasedGlobal(KEY key) const { return input->KeyRelease(key); }
	// --------------------------------------------------------------------------
	void Submit(const IntentWrite& write);

	void SetActiveEntity(EntityID entity)           { inputGate.SetActiveEntity(entity); }
	bool IsManualAllowedFor(EntityID target) const  { return inputGate.Allow(InputChannel::Manual, target); }

	EntityID GetActiveEntity() const                { return inputGate.GetActiveEntity(); }
	void     SetContext(InputContext context)       { inputGate.SetContext(context); }
	InputContext GetContext() const                 { return inputGate.GetContext(); }
	void     SetFocus(FocusState focus)             { inputGate.SetFocus(focus); }
	void     AcquireLock(LockTag tag, int priority) { inputGate.AcquireLock(tag, priority); }
	void     ReleaseLock(LockTag tag)               { inputGate.ReleaseLock(tag); }
	void     SetManualTime(float sec)               { inputGate.SetBlockManualTime(sec); }

	void PushJumpEdge(EntityID target, InputChannel channel = InputChannel::Manual);
	bool ConsumeJumpEdge(EntityID entity);
	void PushAttackEdge(EntityID target, InputChannel channel = InputChannel::Manual);
	bool ConsumeAttackEdge(EntityID entity);

private:
	bool PassPolicy(InputChannel channel, EntityID target) const { return inputGate.Allow(channel, target); }

private:
	SystemRegistry& registry;
	InputMgr*       input{};

	InputGate       inputGate;
	IntentCollector intentCollector;
	unordered_map<EntityID, bool> jumpEdgeByEntity;
	unordered_map<EntityID, bool> attackEdgeByEntity;
};

NS_END