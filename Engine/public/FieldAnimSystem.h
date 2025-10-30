#pragma once

#include "FieldAnimData.h"

NS_BEGIN(Engine)

class ENGINE_DLL FieldAnimSystem : public EntitySystem<LocomotionAnim>, public IGuiRenderable
{
public:
	explicit FieldAnimSystem(SystemRegistry& registry) : EntitySystem(registry){}

	Handle Create(EntityID owner, Handle animHandle);
	Handle Create(EntityID owner, Handle animHandle, const AnimProfile& profile);

	void   Update(float dt);
	void   RenderGui(EntityID id) override;

private:
	const wstring& ResolveClip(const AnimProfile& profile, AnimKey key) const;
	bool  IsCurClipFinished(const LocomotionAnim& loco) const;
	void  PlayKey(LocomotionAnim& loco, AnimKey key, ANIMTYPE type, float fadeSec = 0.f);

private:
	LocoParams params;
};

NS_END