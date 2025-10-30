#pragma once

#include "RunInJobData.h"

NS_BEGIN(Engine)

class ENGINE_DLL RunInSystem : public EntitySystem<RunInJob>
{
public:
	explicit RunInSystem(SystemRegistry& registry) : EntitySystem(registry) {}

	Handle Create(EntityID owner, const RunInJob& job);
	void   Update(float dt);
};

NS_END