#pragma once

#include "TagUtil.h"

NS_BEGIN(Engine)

class ENGINE_DLL TagSystem : public IOwnsEntities
{
public:
	void     Register(EntityID entity, string tag);
	void     Unregister(EntityID entity);
	EntityID Get(TagID tag) const;
	EntityID Get(const string& tag) const;

	void     DestroyOwned(EntityID owner) override;

private:
	// TagID -> EntityID (������ȸ)
	unordered_map<TagID, EntityID> byID;
	// EntityID -> TagID (���� ����/ ����)
	unordered_map<EntityID, TagID> reverse;

#ifdef _DEBUG
	unordered_map<TagID, string> debugMap;
#endif
};

NS_END