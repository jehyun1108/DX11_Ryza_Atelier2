#pragma once

NS_BEGIN(Engine)
class RenderScene;

class ENGINE_DLL IRenderable
{
public:
	virtual ~IRenderable() = default;
	virtual void ExtractRenderProxies(RenderScene& out) const = 0;
};

NS_END