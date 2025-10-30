#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL InputService
{
public:
	void BeginFrame(float dt) { inputGate.Tick(dt); }
	void EndFrameAndApply(SystemRegistry& registry);

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
	InputGate       inputGate;
	IntentCollector intentCollector;
	unordered_map<EntityID, bool> jumpEdgeByEntity;
	unordered_map<EntityID, bool> attackEdgeByEntity;
};

NS_END