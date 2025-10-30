#pragma once

NS_BEGIN(Engine)

// EventInbox 저장/순회 담당
class ENGINE_DLL EventInboxSystem : public EntitySystem<EventInboxData>
{
public:
	explicit EventInboxSystem(SystemRegistry& registry) : EntitySystem(registry) {}

	Handle Create(EntityID owner);
	void   Push(EntityID owner, EventKey key);
	void   ClearAll();
};

NS_END