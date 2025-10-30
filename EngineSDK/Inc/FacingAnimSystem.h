#pragma once

#include "FacingAnimData.h"

NS_BEGIN(Engine)

class ENGINE_DLL FacingAnimSystem : public EntitySystem<FacingState>, public IGuiRenderable
{
public:
    explicit FacingAnimSystem(SystemRegistry& registry) : EntitySystem(registry) {}

    Handle Create(EntityID owner);
    void   EnableOverride(Handle handle, bool enable, const FacingParams* params = {});
    void   Update(float dt);
    void   RenderGui(EntityID id) override;

    void SetDefault(const FacingParams& param) { defaultParams = param; }

private:
    const FacingParams& ParamsFor(const FacingState& state) const { return state.useOverride ? state.params : defaultParams; }

private:
    FacingParams defaultParams{};
};

NS_END