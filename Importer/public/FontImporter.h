#pragma once

#include "FontData.h"

struct FontBasicMetrics
{
    float ascentPx   = 0.f;
    float descentPx  = 0.f;
    float lineHeight = 0.f;
};

struct FontBuildConfig
{
    std::filesystem::path ttfPath;

    float pixelHeight = 32.0f;


    //int atlasWidth = 512;
    //int atlasHeight = 512;

    // 예: ASCII 32~126
    uint32_t firstCodepoint = 0x0020;
    uint32_t lastCodepoint = 0xD7A3;

    int atlasWidth = 4096;
    int atlasHeight = 4096; 
};

struct FontAtlasResult
{
    int atlasWidth  = 0;
    int atlasHeight = 0;

    // RGBA8 로 잡아둘 예정 (나중에 stbi_write_png 로 바로 쓸 수 있게)
    std::vector<unsigned char> pixels; // size = W * H * 4

    FontDesc font; // 엔진 런타임에서 바로 쓸 수 있는 구조
};

bool ComputeFontBasicMetricsFromTTF( const std::filesystem::path& fontPath, float pixelHeight, FontBasicMetrics& outMetrics, std::string& outError);
bool BuildFontAtlasFromTTF( const FontBuildConfig& cfg, FontAtlasResult& outResult, std::string& outError);
bool SaveFontAtlasPNG(const std::filesystem::path& outPath,const FontAtlasResult& atlas,std::string& outError);
bool SaveFontMetaBinary(const std::filesystem::path& outPath,const FontAtlasResult& atlas,std::string& outError);