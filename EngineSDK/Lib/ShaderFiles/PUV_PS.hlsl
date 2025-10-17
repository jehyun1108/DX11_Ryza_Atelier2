#include "Common.hlsli"

Texture2D DiffuseTex : register(t0);
SamplerState DefaultSampler : register(s0);

struct PS_IN
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

float4 PS_MAIN(PS_IN In) : SV_TARGET
{
    return DiffuseTex.Sample(DefaultSampler, In.uv);
}