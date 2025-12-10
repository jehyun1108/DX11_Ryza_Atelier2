#pragma warning(push)
#pragma warning(disable:4996)

#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <vector>
#include <filesystem>
#include <fstream>
#include <cstring>
#include <string>
#include <unordered_map>
using namespace std;
#include "FontImporter.h"

namespace
{
    bool LoadFileBinary(const filesystem::path& path, vector<unsigned char>& out)
    {
        ifstream ifs(path, ios::binary);
        if (!ifs)
            return false;

        ifs.seekg(0, ios::end);
        streamsize size = ifs.tellg();
        ifs.seekg(0, ios::beg);

        if (size <= 0)
            return false;

        out.resize(static_cast<size_t>(size));
        if (!ifs.read(reinterpret_cast<char*>(out.data()), size))
            return false;

        return true;
    }
}

bool ComputeFontBasicMetricsFromTTF( const filesystem::path& fontPath, float pixelHeight, FontBasicMetrics& outMetrics, string& outError)
{
    vector<unsigned char> fontData;
    if (!LoadFileBinary(fontPath, fontData))
    {
        outError = string("Failed to read TTF file: ") + fontPath.string();
        return false;
    }

    stbtt_fontinfo fontInfo{};
    int fontOffset = stbtt_GetFontOffsetForIndex(fontData.data(), 0);
    if (!stbtt_InitFont(&fontInfo, fontData.data(), fontOffset))
    {
        outError = string("stbtt_InitFont failed for: ") + fontPath.string();
        return false;
    }

    float scale = stbtt_ScaleForPixelHeight(&fontInfo, pixelHeight);

    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&fontInfo, &ascent, &descent, &lineGap);

    outMetrics.ascentPx = ascent * scale;
    outMetrics.descentPx = descent * scale;
    outMetrics.lineHeight = (ascent - descent + lineGap) * scale;

    return true;
}

bool BuildFontAtlasFromTTF(const FontBuildConfig& cfg, FontAtlasResult& outResult, string& outError)
{
    outResult = {}; // 결과 초기화

    // 1) TTF 파일 로드 ---------------------------------------------------------
    std::vector<unsigned char> fontData;
    if (!LoadFileBinary(cfg.ttfPath, fontData))
    {
        outError = std::string("Failed to read TTF file: ") + cfg.ttfPath.string();
        return false;
    }

    // 2) stbtt_fontinfo 초기화 -------------------------------------------------
    stbtt_fontinfo fontInfo{};
    int fontOffset = stbtt_GetFontOffsetForIndex(fontData.data(), 0);
    if (!stbtt_InitFont(&fontInfo, fontData.data(), fontOffset))
    {
        outError = std::string("stbtt_InitFont failed for: ") + cfg.ttfPath.string();
        return false;
    }

    // 3) 기본 메트릭 -> FontDesc 에 기록 ---------------------------------------
    float scale = stbtt_ScaleForPixelHeight(&fontInfo, cfg.pixelHeight);

    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&fontInfo, &ascent, &descent, &lineGap);

    outResult.font.name = cfg.ttfPath.stem().wstring(); // 예: "consola"
    outResult.font.atlasNameKey = L"";                          // 나중에 엔진에서 세팅
    outResult.font.ascent = ascent * scale;
    outResult.font.descent = descent * scale;
    outResult.font.lineHeight = (ascent - descent + lineGap) * scale;

    // 4) 아틀라스 알파 버퍼 준비 -----------------------------------------------
    const int atlasW = cfg.atlasWidth;
    const int atlasH = cfg.atlasHeight;

    outResult.atlasWidth = atlasW;
    outResult.atlasHeight = atlasH;

    std::vector<unsigned char> atlasAlpha(atlasW * atlasH);
    std::memset(atlasAlpha.data(), 0, atlasAlpha.size());

    // 5) pack context 만들기 ---------------------------------------------------
    stbtt_pack_context pc{};
    if (!stbtt_PackBegin(&pc, atlasAlpha.data(), atlasW, atlasH, 0, 1, nullptr))
    {
        outError = "stbtt_PackBegin failed (atlas too small?)";
        return false;
    }

    const uint32_t first = cfg.firstCodepoint;
    const uint32_t last = cfg.lastCodepoint;
    const int glyphCount = static_cast<int>(last - first + 1);

    std::vector<stbtt_packedchar> packedChars(glyphCount);

    stbtt_PackSetOversampling(&pc, 1, 1); // 일단 기본 1x

    if (!stbtt_PackFontRange(&pc,
        fontData.data(),
        fontOffset,
        cfg.pixelHeight,
        first,
        glyphCount,
        packedChars.data()))
    {
        stbtt_PackEnd(&pc);
        outError = "stbtt_PackFontRange failed (atlas size too small?)";
        return false;
    }

    stbtt_PackEnd(&pc);

    // 6) packedChars -> FontDesc.glyphs 로 복사 -------------------------------
    outResult.font.glyphs.clear();
    outResult.font.glyphs.reserve(glyphCount);

    const float invW = 1.0f / static_cast<float>(atlasW);
    const float invH = 1.0f / static_cast<float>(atlasH);

    for (int i = 0; i < glyphCount; ++i)
    {
        uint32_t code = first + static_cast<uint32_t>(i);
        const stbtt_packedchar& pcGlyph = packedChars[i];

        Glyph g{};
        g.codePoint = static_cast<char32_t>(code);

        // metrics: px 단위
        g.metrics.offsetX = pcGlyph.xoff;
        g.metrics.offsetY = pcGlyph.yoff;
        g.metrics.advance = pcGlyph.xadvance;
        g.metrics.width =  (float)(pcGlyph.x1 - pcGlyph.x0);
        g.metrics.height = (float)(pcGlyph.y1 - pcGlyph.y0);

        // uv: 0~1 범위로 정규화
        g.uv.u0 = pcGlyph.x0 * invW;
        g.uv.v0 = pcGlyph.y0 * invH;
        g.uv.u1 = pcGlyph.x1 * invW;
        g.uv.v1 = pcGlyph.y1 * invH;

        outResult.font.glyphs.emplace(g.codePoint, g);
    }

    // 7) 알파 → RGBA8 로 변환해서 outResult.pixels 에 저장 ----------------------
    outResult.pixels.resize(static_cast<size_t>(atlasW) * atlasH * 4);

    const int pixelCount = atlasW * atlasH;
    for (int i = 0; i < pixelCount; ++i)
    {
        unsigned char a = atlasAlpha[i];
        const size_t dst = static_cast<size_t>(i) * 4;

        outResult.pixels[dst + 0] = 255; // R
        outResult.pixels[dst + 1] = 255; // G
        outResult.pixels[dst + 2] = 255; // B
        outResult.pixels[dst + 3] = a;   // A
    }

    return true;
}

bool SaveFontAtlasPNG(const std::filesystem::path& outPath, const FontAtlasResult& atlas,string& outError)
{
    if (atlas.atlasWidth <= 0 || atlas.atlasHeight <= 0 || atlas.pixels.empty())
    {
        outError = "SaveFontAtlasPNG: invalid atlas data";
        return false;
    }

    const int w = atlas.atlasWidth;
    const int h = atlas.atlasHeight;
    const int comp = 4;              // RGBA
    const int stride = w * comp;     // 한 줄 바이트 수

    if (!stbi_write_png(outPath.string().c_str(),
        w, h,
        comp,
        atlas.pixels.data(),
        stride))
    {
        outError = "stbi_write_png failed: " + outPath.string();
        return false;
    }

    return true;
}

bool SaveFontMetaBinary(const std::filesystem::path& outPath, const FontAtlasResult& atlas, std::string& outError)
{
    using std::uint32_t;

    const FontDesc& font = atlas.font;

    if (font.glyphs.empty())
    {
        outError = "SaveFontMetaBinary: no glyphs in FontDesc";
        return false;
    }

    std::ofstream ofs(outPath, std::ios::binary);
    if (!ofs)
    {
        outError = "SaveFontMetaBinary: cannot open file: " + outPath.string();
        return false;
    }

    // --- 헤더 작성 -----------------------------------------------------------
    struct Header
    {
        uint32_t magic;
        uint32_t version;
        float    ascent;
        float    descent;
        float    lineHeight;
        uint32_t glyphCount;
    } header{};

    header.magic = '0' | ('F' << 8) | ('N' << 16) | ('T' << 24); // "FNT0"
    header.version = 1;
    header.ascent = font.ascent;
    header.descent = font.descent;
    header.lineHeight = font.lineHeight;
    header.glyphCount = static_cast<uint32_t>(font.glyphs.size());

    ofs.write(reinterpret_cast<const char*>(&header), sizeof(header));

    // --- 글리프 레코드 작성 --------------------------------------------------
    struct GlyphRecord
    {
        uint32_t codePoint;
        float    offsetX, offsetY;
        float    advance;
        float    width, height;
        float    u0, v0, u1, v1;
    };

    for (const auto& kv : font.glyphs)
    {
        const Glyph& g = kv.second;

        GlyphRecord rec{};
        rec.codePoint = static_cast<uint32_t>(g.codePoint);
        rec.offsetX = g.metrics.offsetX;
        rec.offsetY = g.metrics.offsetY;
        rec.advance = g.metrics.advance;
        rec.width = g.metrics.width;
        rec.height = g.metrics.height;
        rec.u0 = g.uv.u0;
        rec.v0 = g.uv.v0;
        rec.u1 = g.uv.u1;
        rec.v1 = g.uv.v1;

        ofs.write(reinterpret_cast<const char*>(&rec), sizeof(rec));
    }

    if (!ofs)
    {
        outError = "SaveFontMetaBinary: write failed: " + outPath.string();
        return false;
    }

    return true;
}
