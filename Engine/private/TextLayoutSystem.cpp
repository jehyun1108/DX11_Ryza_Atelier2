#include "Enginepch.h"
#include "TextLayoutSystem.h"

void TextLayoutSystem::OnBoot()
{
	fontSys = &registry.Get<FontSystem>();
}

void TextLayoutSystem::BuildTextQuads(const TextLayoutDesc& desc, vector<UIDrawItem>& out) const
{
    const FontDesc& font = fontSys->GetFont(desc.fontKey);

    float penX = 0.f;
    float penY = 0.f;

    const float lineHeight = font.lineHeight * desc.scale;
    int glyphIdx = 0;

    for (wchar_t ch : desc.text)
    {
        if (ch == L'\n')
        {
            penX = 0.f;
            penY += lineHeight;
            continue;
        }

        const Glyph& glyph = fontSys->GetGlyph(desc.fontKey, static_cast<char32_t>(ch));
        const float scale = desc.scale;

        const float baseX = desc.originX + (penX + glyph.metrics.offsetX * scale);
        const float baseY = desc.originY + (penY + glyph.metrics.offsetY * scale);
        const float gw = glyph.metrics.width * scale;
        const float gh = glyph.metrics.height * scale;

        // ------------------------------
        // 1) 외곽선 여러 번 찍기
        // ------------------------------
        if (desc.useOutline)
        {
            const float o = desc.outlinePx;

            // 4방향만 써도 꽤 괜찮음 (원하면 8방향으로 늘리면 됨)
            const _float2 offsets[4] =
            {
                { -o, 0.f },
                { o, 0.f },
                { 0.f, -o },
                { 0.f, o },
            };

            for (int i = 0; i < 4; ++i)
            {
                UIDrawItem item{};
                item.zOrder = desc.zOrder + glyphIdx * 2; // 본문보다 살짝 뒤
                item.dstRect.x = baseX + offsets[i].x;
                item.dstRect.y = baseY + offsets[i].y;
                item.dstRect.width = gw;
                item.dstRect.height = gh;

                item.texKey = font.atlasNameKey;
                item.alpha = desc.alpha;
                item.color = desc.outlineColor;    // 여기 중요
                item.useScissor = false;
                item.scissorRect = {};
                item.rotDeg = 0.f;
                item.pivotNX = 0.f;
                item.pivotNY = 0.f;
                item.fillRatioX = 1.f;
                item.fillRatioY = 1.f;
                item.fillMode = UIFillMode::Rect;
                item.flipMode = UIFlipMode::None;
                item.maskType = UIMaskType::None;

                item.srcU0 = glyph.uv.u0;
                item.srcV0 = glyph.uv.v0;
                item.srcU1 = glyph.uv.u1;
                item.srcV1 = glyph.uv.v1;

                out.push_back(item);
            }
        }

        // ------------------------------
        // 2) 본문 글자 한 번
        // ------------------------------
        {
            UIDrawItem item{};
            item.zOrder = desc.zOrder + glyphIdx * 2 + 1; // 외곽선 앞에 오도록 +1
            item.dstRect.x = baseX;
            item.dstRect.y = baseY;
            item.dstRect.width = gw;
            item.dstRect.height = gh;

            item.texKey = font.atlasNameKey;
            item.alpha = desc.alpha;
            item.color = desc.textColor;
            item.useScissor = false;
            item.scissorRect = {};
            item.rotDeg = 0.f;
            item.pivotNX = 0.f;
            item.pivotNY = 0.f;
            item.fillRatioX = 1.f;
            item.fillRatioY = 1.f;
            item.fillMode = UIFillMode::Rect;
            item.flipMode = UIFlipMode::None;
            item.maskType = UIMaskType::None;

            item.srcU0 = glyph.uv.u0;
            item.srcV0 = glyph.uv.v0;
            item.srcU1 = glyph.uv.u1;
            item.srcV1 = glyph.uv.v1;

            out.push_back(item);
        }

        penX += glyph.metrics.advance * scale;
        ++glyphIdx;
    }
}


TextBounds TextLayoutSystem::Measure(const TextLayoutDesc& desc) const
{
	const FontDesc& font = fontSys->GetFont(desc.fontKey);

	float penX = 0.f;
	float penY = 0.f;

	float maxLineW = 0.f;
	const float lineH = font.lineHeight * desc.scale;

	for (wchar_t ch : desc.text)
	{
		if (ch == L'\n')
		{
			if (penX > maxLineW) maxLineW = penX;
			penX = 0.f;
			penY += lineH;
			continue;
		}

		const Glyph& g = fontSys->GetGlyph(desc.fontKey, static_cast<char32_t>(ch));
		penX += g.metrics.advance * desc.scale;
	}

	if (penX > maxLineW) maxLineW = penX;

	TextBounds b{};
	b.width = maxLineW;
	b.height = penY + lineH;
	return b;
}