#pragma once

#include "FacingData.h"

NS_BEGIN(Engine)

class ENGINE_DLL FacingSystem : public EntitySystem<FacingComponent>
{
public:
    explicit FacingSystem(SystemRegistry& registry) : EntitySystem(registry) {}
    void     OnBoot() override;

    Handle Create(EntityID owner);
    void   Update(float dt);

private:
    TransformSystem*    tfSys{};
    MoveStateSystem*    moveSys{};
    MoveIntentSystem*   intentSys{};
    FacingForceService* forceSrv{};
    FacingBlockService* blockSrv{};
};

NS_END