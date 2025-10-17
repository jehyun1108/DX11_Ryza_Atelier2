#pragma once

NS_BEGIN(Engine)

using TagID = _uint;

inline constexpr TagID FNV1a_32(const char* str, size_t count) {
    return ((count ? FNV1a_32(str, count - 1) : 2166136261u) ^ static_cast<unsigned char>(str[count])) * 16777619u;
}

inline constexpr TagID operator"" _tag(const char* c, size_t count) { return FNV1a_32(c, count - 1); }

inline TagID HashTag(const string& str)
{
    TagID hash = 2166136261u;
    for (unsigned char ch : str)
    {
        hash ^= ch;
        hash *= 16777619u;
    }
    return hash;
}

NS_END