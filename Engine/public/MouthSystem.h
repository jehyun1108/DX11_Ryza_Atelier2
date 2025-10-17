#pragma once

#include "MouthData.h"

NS_BEGIN(Engine)

class ENGINE_DLL MouthSystem : public EntitySystem<MouthData>, public IGuiRenderable
{
public:
	explicit MouthSystem(SystemRegistry& registry) : EntitySystem(registry) {}

	Handle Create(EntityID owner, Handle anim, wstring clip, _uint layer = 2,
		float weight = 1.f, float speed = 1.f);

	void SetWeight(Handle handle, float weight);
	void SetSpeed(Handle handle, float speed);
	void SetClip(Handle handle, const wstring& clip);

	void RenderGui(EntityID id) override;
};

NS_END