#pragma once

#include "ModelData.h"

NS_BEGIN(Engine)

class ENGINE_DLL ModelSystem : public EntitySystem<ModelData>, public IGuiRenderable
{
public:
	explicit ModelSystem(SystemRegistry& registry) : EntitySystem(registry) {}
	void     OnBoot() override;
	Handle   Create(EntityID owner, Handle transform, const wstring& modelKey, Handle animHandle = {});
	void SetEnabled(Handle handle, bool on);

	void RenderGui(EntityID id) override;

private:
	AssetSystem*        assets{};
	AnimatorSystem*     animator{};
	MeshColliderSystem* mcSys{};
};

NS_END