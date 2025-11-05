#include "Enginepch.h"

static inline pair<float, float> ToNorm(UIAnchor anchor)
{
	switch (anchor)
	{
	case UIAnchor::TopLeft:      return { 0.f,  0.f  };
	case UIAnchor::TopCenter:    return { 0.5f, 0.f  };
	case UIAnchor::TopRight:     return { 1.f,  0.f  };
	case UIAnchor::MidLeft:      return { 0.f,  0.5f };
	case UIAnchor::MidCenter:    return { 0.5f, 0.5f };
	case UIAnchor::MidRight:     return { 1.f,  0.5f };
	case UIAnchor::BottomLeft:   return { 0.f,  1.f  };
	case UIAnchor::BottomCenter: return { 0.5f, 1.f  };
	case UIAnchor::BottomRight:  return { 1.f,  1.f  };
	}
	return { 0.5f, 0.5f };
}

static inline pair<float, float> ToNorm(UIPivot pivot)
{
	switch (pivot)
	{
	case UIPivot::TopLeft:      return { 0.f,  0.f  };
	case UIPivot::TopCenter:    return { 0.5f, 0.f  };
	case UIPivot::TopRight:     return { 1.f,  0.f  };
	case UIPivot::MidLeft:      return { 0.f,  0.5f };
	case UIPivot::MidCenter:    return { 0.5f, 0.5f };
	case UIPivot::MidRight:     return { 1.f,  0.5f };
	case UIPivot::BottomLeft:   return { 0.f,  1.f  };
	case UIPivot::BottomCenter: return { 0.5f, 1.f  };
	case UIPivot::BottomRight:  return { 1.f,  1.f  };
	}
	return { 0.5f, 0.5f };
}
// ------------------------------------------------------------------------------------------------------------------

void UISystem::OnBoot()
{
	assets     = &registry.Get<AssetSystem>();
	uiRegistry = &registry.Get<UIRegistry>();
	uiAnimSys  = &registry.Get<UIAnimSystem>();
	director   = &registry.Get<GameModeDirectorSystem>();

	assert(assets && uiRegistry && uiAnimSys && director);
}

void UISystem::Tick(float dt)
{

}

void UISystem::ExtractUIProxies(UISnapShot& out)
{
	out.drawItems.clear();
	out.drawItems.reserve(64);

	const UIContext activeContext = (director->GetMode() == GameMode::Battle) ? UIContext::Battle : UIContext::Field;

	vector<const UIInstance*> candidates;
	uiRegistry->CollectForContext(activeContext, candidates);

	const auto& viewport = GAME.GetViewport();
	const float screenW = static_cast<float>(viewport.Width);
	const float screenH = static_cast<float>(viewport.Height);

	for (const UIInstance* inst : candidates)
	{
		const UIArchetypeSpec& spec = *inst->spec;

		auto [srcW, srcH] = uiRegistry->GetOrCacheTexSize(spec.texKey);

		float drawW = srcW, drawH = srcH;
		switch (spec.sizeMode)
		{
		case UISizeMode::Original:  
			break;
		case UISizeMode::Fixed:   
			drawW = spec.fixedWidth;    
			drawH = spec.fixedHeight; 
			break;
		case UISizeMode::Ratio:  
			drawW = srcW * spec.ratioX; 
			drawH = srcH * spec.ratioY; 
			break;
		}
		// Anchor
		auto [anchorNX, anchorNY] = ToNorm(spec.anchor);
		const float anchorX       = anchorNX * screenW;
		const float anchorY       = anchorNY * screenH;
		// Pivot
		auto [pivotNX, pivotNY] = ToNorm(spec.pivot);
		const float pivotOffX   = drawW * pivotNX;
		const float pivotOffY   = drawH * pivotNY;

		const float topLeftX = anchorX + inst->localX - pivotOffX + inst->animOffsetX;
		const float topLeftY = anchorY + inst->localY - pivotOffY + inst->animOffsetY;

		UIDrawItem drawItem{};
		drawItem.zOrder         = (spec.zOrder + inst->zOrder);
		drawItem.dstRect.x      = topLeftX;
		drawItem.dstRect.y      = topLeftY;
		drawItem.dstRect.width  = drawW * inst->animScaleX;
		drawItem.dstRect.height = drawH * inst->animScaleY;
		drawItem.texKey         = inst->overrideKey.has_value() ? *inst->overrideKey : spec.texKey;

		drawItem.useScissor  = inst->useScissor;
		drawItem.scissorRect = inst->scissorRect;

		out.drawItems.push_back(drawItem);
	}
}