#include "Enginepch.h"

void FacingBlockService::Block(EntityID entity, FacingBlockReason reason)
{
	++blockCountByEntity[entity];
}

void FacingBlockService::UnBlock(EntityID entity, FacingBlockReason reason)
{
	auto it = blockCountByEntity.find(entity);
	if (it == blockCountByEntity.end()) return;
	if (it->second > 0) --it->second;
	if (it->second == 0) blockCountByEntity.erase(it);
}

bool FacingBlockService::IsBlocked(EntityID entity) const
{
	auto it = blockCountByEntity.find(entity);
	return (it != blockCountByEntity.end() && it->second > 0);
}