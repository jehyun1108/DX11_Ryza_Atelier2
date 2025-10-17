#pragma once

#include "LayerData.h"

NS_BEGIN(Engine)

class ENGINE_DLL LayerSystem : public EntitySystem<LayerData>, public IGuiRenderable
{
public:
	explicit LayerSystem(SystemRegistry& registry) : EntitySystem(registry) {}

	Handle Create(EntityID owner, Handle transform, _uint mask);

	void  SetMask(Handle handle, _uint mask);
	void  Enable(Handle handle, bool on);
	_uint GetMask(Handle handle) const;

	void RenderGui(EntityID id) override;

	template<typename Fn>
	void ForEach(_uint visibleMask, Fn&& fn) const
	{
		ForEachAlive([&](_uint, const LayerData& layer)
			{
				if (!layer.enabled) return;
				if ((layer.layerMask & visibleMask) == 0) return;
				fn(layer);
			});
	}

	template<typename Fn>
	void ForEachByMask(_uint mask, Fn&& fn) const
	{
		ForEachAliveEx([&](Handle handle, EntityID owner, const LayerData& layer)
			{
				if (!layer.enabled) return;
				if ((layer.layerMask & mask) == 0) return;
				fn(owner, handle, layer);
			});
	}
};

NS_END