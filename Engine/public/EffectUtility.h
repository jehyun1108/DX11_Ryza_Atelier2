#pragma once

NS_BEGIN(Engine)

enum class EffectCurveType
{
    Linear, EaseIn, EaseOut, EaseInOut, Spike, Bell, Plateau    
};
enum class EffectPresetType
{
    None, Spark, Smoke, HitFlash, Ring
};

class ENGINE_DLL EffectUtility
{
public:
    static float Curve(EffectCurveType type, float t);
    static void ApplyPreset(struct ParticleSpawnData& spawnData, EffectPresetType preset);

#ifdef USE_IMGUI
    static void DrawCurvePreview(const char* label, EffectCurveType type, const ImVec2& size = ImVec2(80.f, 40.f));
#endif
};

NS_END