#pragma once

#include "FacingBlockData.h"

NS_BEGIN(Engine)

class ENGINE_DLL FacingBlockService
{
public:
	void Block(EntityID entity, FacingBlockReason reason);
	void UnBlock(EntityID entity, FacingBlockReason reason);
	bool IsBlocked(EntityID entity) const;

private:
	unordered_map<EntityID, int> blockCountByEntity;
};

NS_END