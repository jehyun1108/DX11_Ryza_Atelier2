#pragma once

#include "FacingBlockData.h"

NS_BEGIN(Engine)

class ENGINE_DLL FacingBlockService : public ISystem
{
public:
	explicit FacingBlockService(SystemRegistry& registry) : registry(registry) {}

	void Block(EntityID entity, FacingBlockReason reason);
	void UnBlock(EntityID entity, FacingBlockReason reason);
	bool IsBlocked(EntityID entity) const;

private:
	SystemRegistry& registry;
	unordered_map<EntityID, int> blockCountByEntity;
};

NS_END