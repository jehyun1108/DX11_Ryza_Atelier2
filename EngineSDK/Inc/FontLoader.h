#pragma once

NS_BEGIN(Engine)

class ENGINE_DLL FontLoader 
{
public:
    static bool LoadFontDescFromBinary(const filesystem::path& path, FontDesc& outFont, string& outError);
};

NS_END