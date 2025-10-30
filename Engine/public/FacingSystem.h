#pragma once

#include "FacingData.h"

NS_BEGIN(Engine)

class ENGINE_DLL FacingSystem : public EntitySystem<FacingComponent>
{
public:
    explicit FacingSystem(SystemRegistry& registry) : EntitySystem(registry) {}

    Handle Create(EntityID owner);
    void   Update(float dt);
};

NS_END