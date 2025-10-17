#pragma once

#include "FaceData.h"

NS_BEGIN(Engine)

class ENGINE_DLL FaceSystem : public EntitySystem<FaceData>, public IGuiRenderable
{
public:
    explicit FaceSystem(SystemRegistry& registry) : EntitySystem(registry) {}

    Handle Create(EntityID owner, Handle anim, wstring openClip, wstring closeClip,
        float openDur = 2.5f, float openJitter = 1.2f,
        float holdClose = 0.12f, float fadeClose = 0.08f, float fadeOpen = 0.08f);

    void SetSpeed(Handle handle, float speed);
    void Update(float dt);

    void RenderGui(EntityID id) override;

private:
    static float nextOpenHold(const FaceData& face);
};

NS_END