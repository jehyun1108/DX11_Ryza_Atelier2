#include "Common.hlsli"

Texture2D DiffuseTex : register(t0);
SamplerState DefaultSampler : register(s0);

struct PS_IN
{
    float4 pos : SV_POSITION;
    float3 worldPos : POSITION;
    float3 worldNormal : NORMAL;
    float3 worldTangent : TANGENT;
    float2 uv : TEXCOORD0;
};

float3 ComputeDirectional(float3 N, float3 V, float3 albedo)
{
    float3 L = normalize(-light.lightDir.xyz);
    float ndl = saturate(dot(N, L));
    float3 diffuse = light.diffuse.rgb * ndl * albedo;

    // 간단한 Blinn-Phong 하이라이트
    float3 H = normalize(L + V);
    float ndh = saturate(dot(N, H));
    float3 spec = light.specular.rgb * pow(ndh, 32.0f);

    return diffuse + spec;
}

float4 PS_MAIN(PS_IN In) : SV_TARGET
{
    float4 tex = DiffuseTex.Sample(DefaultSampler, In.uv);
    float3 N = normalize(In.worldNormal);
    float3 V = normalize(camPos.xyz - In.worldPos);


    float3 color = tex.rgb * light.ambient.rgb;
    color += ComputeDirectional(N, V, tex.rgb);

    return float4(color, tex.a);
}