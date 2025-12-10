#include "Enginepch.h"

static inline UIContext ToUIContext(GameMode mode)
{
	switch (mode)
	{
	case GameMode::Battle: return UIContext::Battle;
	case GameMode::Field:  return UIContext::Field;
	case GameMode::Menu:   return UIContext::Logo; 
	}
	return UIContext::Field;
}
void UISystem::OnBoot()
{
	assets        = &registry.Get<AssetSystem>();
	uiRegistry    = &registry.Get<UIRegistry>();
	uiAnimSys     = &registry.Get<UIAnimSystem>();
	director      = &registry.Get<GameModeDirectorSystem>();
	textLayoutSys = &registry.Get<TextLayoutSystem>();
}

void UISystem::ExtractUIProxies(UISnapShot& out)
{
	out.drawItems.clear();
	out.drawItems.reserve(256);

	const UIContext ctx = activeContext;

	vector<const UIInstance*> candidates;
	uiRegistry->CollectForContext(ctx, candidates);
	uiRegistry->CollectForContext(UIContext::Always, candidates);

	const auto& viewport = GAME.GetViewport();
	const float screenW = static_cast<float>(viewport.Width);
	const float screenH = static_cast<float>(viewport.Height);

	for (const UIInstance* inst : candidates)
	{
		const UIArchetypeSpec& spec = *inst->spec;
		if (spec.widgetType == UIWidgetType::Image)
		{
			const wstring effectiveTexKey = inst->overrideKey.has_value() ? *inst->overrideKey : spec.texKey;
			auto [srcW, srcH] = uiRegistry->GetOrCacheTexSize(effectiveTexKey);

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

			auto [anchorNX, anchorNY] = ToNorm(spec.anchor);
			const float anchorX = anchorNX * screenW;
			const float anchorY = anchorNY * screenH;

			const float scaledW = drawW * inst->animScaleX;
			const float scaledH = drawH * inst->animScaleY;

			auto [pivotNX, pivotNY] = ToNorm(spec.pivot);
			const float pivotOffX = scaledW * pivotNX;
			const float pivotOffY = scaledH * pivotNY;

			const float topLeftX = anchorX + inst->localX - pivotOffX + inst->animOffsetX;
			const float topLeftY = anchorY + inst->localY - pivotOffY + inst->animOffsetY;

			UIDrawItem drawItem{};
			drawItem.zOrder          = (spec.zOrder + inst->zOrder);
			drawItem.dstRect.x       = topLeftX;
			drawItem.dstRect.y       = topLeftY;
			drawItem.dstRect.width   = scaledW;
			drawItem.dstRect.height  = scaledH;
			drawItem.texKey          = effectiveTexKey;
			drawItem.rotDeg          = inst->animRotDeg;
			drawItem.pivotNX         = pivotNX;
			drawItem.pivotNY         = pivotNY;
			drawItem.useScissor      = inst->useScissor;
			drawItem.scissorRect     = inst->scissorRect;
			drawItem.alpha           = inst->animAlpha;
			drawItem.fillRatioX      = inst->fillRatioX;
			drawItem.fillRatioY      = inst->fillRatioY;
			drawItem.fillMode        = spec.fillMode;
			drawItem.flipMode        = inst->flipMode;
			drawItem.maskType        = inst->spec->maskType;
			drawItem.color           = spec.imageColor;

			out.drawItems.push_back(drawItem);
		}
		else if (spec.widgetType == UIWidgetType::Text)
		{
			const wstring& fontKey = inst->fontKey.empty() ? spec.fontKey : inst->fontKey;

			TextLayoutDesc measureDesc{};
			measureDesc.fontKey = fontKey;
			measureDesc.text  = inst->text;
			measureDesc.scale = inst->animScaleX; 

			TextBounds bounds = textLayoutSys->Measure(measureDesc);
			float textW = bounds.width;
			float textH = bounds.height;
			float boxW  = textW;
			float boxH  = textH;

			if (spec.sizeMode == UISizeMode::Fixed)
			{
				if (spec.fixedWidth > 0.f) boxW = spec.fixedWidth;
				if (spec.fixedHeight > 0.f) boxH = spec.fixedHeight;
			}
			else if (spec.sizeMode == UISizeMode::Ratio)
			{
				boxW = textW * spec.ratioX;
				boxH = textH * spec.ratioY;
			}

			auto [anchorNX, anchorNY] = ToNorm(spec.anchor);
			float anchorX = anchorNX * screenW;
			float anchorY = anchorNY * screenH;

			auto [pivotNX, pivotNY] = ToNorm(spec.pivot);
			float pivotOffX = boxW * pivotNX;
			float pivotOffY = boxH * pivotNY;

			float boxLeft = anchorX + inst->localX - pivotOffX + inst->animOffsetX;
			float boxTop = anchorY + inst->localY - pivotOffY + inst->animOffsetY;

			float originX = boxLeft;
			float originY = boxTop;

			switch (spec.alignH)
			{
			case UITextAlignHorizontal::Left:
				originX = boxLeft;
				break;
			case UITextAlignHorizontal::Center:
				originX = boxLeft + (boxW - textW) * 0.5f;
				break;
			case UITextAlignHorizontal::Right:
				originX = boxLeft + (boxW - textW);
				break;
			}

			switch (spec.alignV)
			{
			case UITextAlignVertical::Top:
				originY = boxTop;
				break;
			case UITextAlignVertical::Mid:
				originY = boxTop + (boxH - textH) * 0.5f;
				break;
			case UITextAlignVertical::Bottom:
				originY = boxTop + (boxH - textH);
				break;
			}

			TextLayoutDesc drawDesc{};
			drawDesc.fontKey      = fontKey;
			drawDesc.text         = inst->text;
			drawDesc.originX      = originX;
			drawDesc.originY      = originY;
			drawDesc.scale        = inst->animScaleX;
			drawDesc.zOrder       = spec.zOrder + inst->zOrder;
			drawDesc.alpha        = inst->animAlpha;
			drawDesc.useOutline   = spec.useOutline;
			drawDesc.outlinePx    = spec.outlinePx;
			drawDesc.textColor    = spec.textColor;
			drawDesc.outlineColor = spec.outlineColor;

			textLayoutSys->BuildTextQuads(drawDesc, out.drawItems);
		}
	}
}

void UISystem::SetText(const wstring& key, float x, float y, const wstring& text)
{
	UIInstance& inst = uiRegistry->Ensure(key);

	inst.selfEnabled = true;
	inst.text = text;
	inst.localX = x;
	inst.localY = y;
}

void UISystem::SetText(const wstring& key, const _float2& pos, const wstring& text)
{
	SetText(key, pos.x, pos.y, text);
}

void UISystem::SetText(const wstring& key, const wstring& text)
{
	UIInstance& inst = uiRegistry->Ensure(key);

	inst.selfEnabled = true;
	inst.text = text;
}

pair<float, float> UISystem::ToNorm(UIAnchor anchor)
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

pair<float, float> UISystem::ToNorm(UIPivot pivot)
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