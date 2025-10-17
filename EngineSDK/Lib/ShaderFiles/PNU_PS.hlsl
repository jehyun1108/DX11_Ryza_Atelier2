#include "Common.hlsli"

Texture2D DiffuseTex : register(t0);
SamplerState DefaultSampler : register(s0);

struct PS_IN
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 worldNormal : NORMAL;
    float3 worldPos : TEXCOORD1;
};

float4 PS_MAIN(PS_IN In) : SV_TARGET
{
    float4 texColor = DiffuseTex.Sample(DefaultSampler, In.uv);
    float3 normal = normalize(In.worldNormal);
    float3 viewDir = normalize(camPos.xyz - In.worldPos);
    
    float3 finalColor = texColor.rgb * light.ambient.rgb;
    
    float3 lightDir = (float3) 0.f;
    float attenuation = 1.f;

    if (light.type == LIGHT_DIRECTIONAL)
        lightDir = normalize(-light.lightDir.xyz);

    float diffuseRatio = saturate(dot(lightDir, normal));
    float3 diffuseColor = light.diffuse.rgb * diffuseRatio * texColor.rgb;
    
    float3 reflectDir = reflect(-lightDir, normal);
    float specularRatio = pow(saturate(dot(viewDir, reflectDir)), 32.f);
    float3 specularColor = light.specular.rgb * specularRatio;
    
    finalColor += (diffuseColor + specularColor) * attenuation;
    finalColor *= 0.8f;

    return float4(finalColor, texColor.a);
}
