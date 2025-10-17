#pragma once

#include "ModelData.h"

NS_BEGIN(Engine)

class ENGINE_DLL ModelSystem : public EntitySystem<ModelData>, public IGuiRenderable
{
public:
	explicit ModelSystem(SystemRegistry& registry) : EntitySystem(registry) {}

	Handle Create(EntityID owner, Handle transform, const wstring& modelKey, Handle animator = {});
	void SetEnabled(Handle handle, bool on);

	void RenderGui(EntityID id) override;
};

NS_END