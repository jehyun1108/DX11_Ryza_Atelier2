#include "Enginepch.h"

float EffectUtility::Curve(EffectCurveType type, float t)
{
    t = Utility::Saturate(t);

    switch (type)
    {
    case EffectCurveType::Linear:
        return t;

    case EffectCurveType::EaseIn:          // 처음 천천히, 나중에 빨리
        return t * t;                      // t^2

    case EffectCurveType::EaseOut:         // 처음 빨리, 나중에 천천히
    {
        float u = 1.f - t;
        return 1.f - u * u;                // 1 - (1-t)^2
    }

    case EffectCurveType::EaseInOut:
    {
        if (t < 0.5f)
        {
            float u = t * 2.f;             // [0,1]
            return 0.5f * u * u;           // 앞쪽 EaseIn
        }
        else
        {
            float u = (1.f - t) * 2.f;     // [0,1]
            return 1.f - 0.5f * u * u;     // 뒤쪽 EaseOut
        }
    }

    case EffectCurveType::Spike:           // /\ 모양
    {
        float u = 1.f - fabsf(2.f * t - 1.f); // 가운데에서 1, 양끝 0
        return u;
    }

    case EffectCurveType::Bell:            // 부드러운 종 모양
    {
        float x = 2.f * t - 1.f;           // [-1,1]
        float y = 1.f - x * x;             // 위로 볼록 포물선
        return Utility::Saturate(y);
    }

    case EffectCurveType::Plateau:         // 가운데 평평한 펄스
    {
        float edge = 0.2f;                 // 앞뒤 20%는 부드럽게 오르내리기
        if (t < edge)
        {
            float u = t / edge;            // [0,1]
            return u * u;                  // EaseIn
        }
        else if (t > 1.f - edge)
        {
            float u = (1.f - t) / edge;    // [0,1]
            return u * u;                  // EaseOut (역방향)
        }
        else
            return 1.f;                    // 가운데 평평
    }
    }

    return t;
}

void EffectUtility::ApplyPreset(ParticleSpawnData& sd, EffectPresetType preset)
{
    // basic defaults
    sd.spawnRate  = 10.f;
    sd.lifeMin    = 0.5f;
    sd.lifeMax    = 1.0f;
    sd.speedMin   = 5.f;
    sd.speedMax   = 10.f;
    sd.baseDir    = _float3(0.f, 1.f, 0.f);
    sd.spreadAng  = XM_PIDIV4;
    sd.startSize  = 30.f;
    sd.endSize    = 60.f;
    sd.startColor = _float4(1.f, 1.f, 1.f, 1.f);
    sd.endColor   = _float4(1.f, 1.f, 1.f, 0.f);
    sd.sizeCurve  = EffectCurveType::Linear;
    sd.alphaCurve = EffectCurveType::EaseOut;
    sd.colorCurve = EffectCurveType::Linear;
    sd.rateCurve  = EffectCurveType::Linear;
    // sd.texKey는 상황에 맞게 너가 따로 맞춰줘도 됨

    switch (preset)
    {
    case EffectPresetType::Spark:
        sd.spawnRate  = 80.f;
        sd.lifeMin    = 0.15f;
        sd.lifeMax    = 0.30f;
        sd.speedMin   = 15.f;
        sd.speedMax   = 30.f;
        sd.baseDir    = _float3(0.f, 1.f, 0.f);
        sd.spreadAng  = XM_PIDIV2;                // wide cone
        sd.startSize  = 15.f;
        sd.endSize    = 10.f;
        sd.startColor = _float4(1.f, 0.9f, 0.6f, 1.f);
        sd.endColor   = _float4(1.f, 0.4f, 0.0f, 0.f);
        sd.sizeCurve  = EffectCurveType::EaseOut;
        sd.alphaCurve = EffectCurveType::EaseOut;
        sd.colorCurve = EffectCurveType::EaseOut;
        sd.rateCurve  = EffectCurveType::Spike;
        // sd.texKey   = L"fx_spark"; // 원하는 키로
        break;

    case EffectPresetType::Smoke:
        sd.spawnRate  = 25.f;
        sd.lifeMin    = 1.0f;
        sd.lifeMax    = 2.5f;
        sd.speedMin   = 1.5f;
        sd.speedMax   = 3.0f;
        sd.baseDir    = _float3(0.f, 1.f, 0.f);
        sd.spreadAng  = XM_PIDIV4;
        sd.startSize  = 40.f;
        sd.endSize    = 90.f;
        sd.startColor = _float4(0.4f, 0.4f, 0.4f, 0.0f);
        sd.endColor   = _float4(0.2f, 0.2f, 0.2f, 0.0f);
        sd.sizeCurve  = EffectCurveType::EaseOut;
        sd.alphaCurve = EffectCurveType::Plateau;
        sd.colorCurve = EffectCurveType::EaseOut;
        sd.rateCurve  = EffectCurveType::Linear;
        // sd.texKey   = L"fx_smoke";
        break;

    case EffectPresetType::HitFlash:
        sd.spawnRate  = 1.f;      // usually burst
        sd.lifeMin    = 0.05f;
        sd.lifeMax    = 0.10f;
        sd.speedMin   = 0.f;
        sd.speedMax   = 0.f;
        sd.baseDir    = _float3(0.f, 0.f, 1.f);
        sd.spreadAng  = 0.f;
        sd.startSize  = 60.f;
        sd.endSize    = 80.f;
        sd.startColor = _float4(1.f, 1.f, 1.f, 1.f);
        sd.endColor   = _float4(1.f, 1.f, 1.f, 0.f);
        sd.sizeCurve  = EffectCurveType::EaseOut;
        sd.alphaCurve = EffectCurveType::Spike;
        sd.colorCurve = EffectCurveType::Linear;
        sd.rateCurve  = EffectCurveType::Spike;
        // sd.texKey   = L"fx_hitflash";
        break;

    case EffectPresetType::Ring:
        sd.spawnRate  = 1.f;
        sd.lifeMin    = 0.4f;
        sd.lifeMax    = 0.6f;
        sd.speedMin   = 0.f;
        sd.speedMax   = 0.f;
        sd.baseDir    = _float3(0.f, 1.f, 0.f);
        sd.spreadAng  = 0.f;
        sd.startSize  = 20.f;
        sd.endSize    = 100.f;
        sd.startColor = _float4(1.f, 0.8f, 0.4f, 1.f);
        sd.endColor   = _float4(1.f, 0.8f, 0.4f, 0.f);
        sd.sizeCurve  = EffectCurveType::EaseOut;
        sd.alphaCurve = EffectCurveType::EaseOut;
        sd.colorCurve = EffectCurveType::EaseOut;
        sd.rateCurve  = EffectCurveType::Linear;
        // sd.texKey   = L"fx_ring";
        break;

    case EffectPresetType::None:
    default:
        break;
    }
}

#ifdef USE_IMGUI
void EffectUtility::DrawCurvePreview(const char* label, EffectCurveType type, const ImVec2& size)
{
    constexpr int sampleCount = 64;
    float samples[sampleCount];
    for (int i = 0; i < sampleCount; ++i)
    {
        float t = (float)i / (float)(sampleCount - 1); // 0~1
        samples[i] = Curve(type, t);
    }
    // y 범위 [0,1] 고정
    ImGui::PlotLines(label, samples, sampleCount, 0, nullptr, 0.0f, 1.0f, size);
}
#endif