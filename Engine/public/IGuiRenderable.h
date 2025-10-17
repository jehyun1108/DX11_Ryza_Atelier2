#pragma once

NS_BEGIN(Engine)

struct ENGINE_DLL IGuiRenderable
{
	virtual ~IGuiRenderable() = default;
	virtual void RenderGui(EntityID id) = 0;
};

NS_END