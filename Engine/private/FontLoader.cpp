#include "Enginepch.h"
#include "FontLoader.h"

#pragma pack(push, 1)
struct FontFileHeader
{
    uint32_t magic;       // "FNT0"
    uint32_t version;
    float    ascent;
    float    descent;
    float    lineHeight;
    uint32_t glyphCount;
};

struct FontFileGlyphRecord
{
    uint32_t codePoint;
    float    offsetX, offsetY;
    float    advance;
    float    width, height;
    float    u0, v0, u1, v1;
};
#pragma pack(pop)

bool FontLoader::LoadFontDescFromBinary(const filesystem::path& path, FontDesc& outFont, string& outError)
{
    ifstream ifs(path, ios::binary);
    if (!ifs)
    {
        outError = "LoadFontDescFromBinary: cannot open file: " + path.string();
        return false;
    }

    FontFileHeader hdr{};
    if (!ifs.read(reinterpret_cast<char*>(&hdr), sizeof(hdr)))
    {
        outError = "LoadFontDescFromBinary: failed to read header: " + path.string();
        return false;
    }

    const uint32_t expectedMagic =
        ('0') | ('F' << 8) | ('N' << 16) | ('T' << 24); // Save 쪽과 동일하게

    if (hdr.magic != expectedMagic)
    {
        outError = "LoadFontDescFromBinary: invalid magic: " + path.string();
        return false;
    }

    if (hdr.version != 1)
    {
        outError = "LoadFontDescFromBinary: unsupported version: " + path.string();
        return false;
    }

    // FontDesc 기본값 세팅
    outFont.glyphs.clear();
    outFont.ascent = hdr.ascent;
    outFont.descent = hdr.descent;
    outFont.lineHeight = hdr.lineHeight;

    // 나중에 호출 쪽에서 채울 예정이라 여기서는 비워둠
    outFont.name.clear();
    outFont.atlasNameKey.clear();

    for (uint32_t i = 0; i < hdr.glyphCount; ++i)
    {
        FontFileGlyphRecord rec{};
        if (!ifs.read(reinterpret_cast<char*>(&rec), sizeof(rec)))
        {
            outError = "LoadFontDescFromBinary: failed to read glyph records: " + path.string();
            return false;
        }

        Glyph g{};
        g.codePoint = static_cast<char32_t>(rec.codePoint);
        g.metrics.offsetX = rec.offsetX;
        g.metrics.offsetY = rec.offsetY;
        g.metrics.advance = rec.advance;
        g.metrics.width = rec.width;
        g.metrics.height = rec.height;
        g.uv.u0 = rec.u0;
        g.uv.v0 = rec.v0;
        g.uv.u1 = rec.u1;
        g.uv.v1 = rec.v1;

        outFont.glyphs.emplace(g.codePoint, g);
    }

    if (!ifs)
    {
        outError = "LoadFontDescFromBinary: stream error after reading glyphs: " + path.string();
        return false;
    }

    return true;
}
