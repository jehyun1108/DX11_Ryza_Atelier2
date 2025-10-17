#include "Common.hlsli"

struct VS_IN
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
    float3 tangent : TANGENT;
    uint4 blendIndices : BLENDINDICES0;
    float4 blendWeights : BLENDWEIGHT0;
};

struct VS_OUT
{
    float4 pos : SV_POSITION;
    float3 worldPos : POSITION;
    float3 worldNormal : NORMAL;
    float2 uv : TEXCOORD0;
    float3 worldTangent : TANGENT;
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out = (VS_OUT) 0;

    matrix skinMat = 0;
    float weightSum = 0.f;
    
    [unroll]
    for (int i = 0; i < 4; ++i)
    {
        const float w = In.blendWeights[i];
        if (w > 0.f)
        {
            skinMat += boneMat[In.blendIndices[i]] * w;
            weightSum += w;
        }
    }
    
    if (weightSum < 1e-6)
        skinMat = (matrix) 1;
    else if (abs(1.f - weightSum) > 1e-3f)
        skinMat /= weightSum;
    
    float4 worldPos = mul(float4(In.pos, 1.f), skinMat);
    worldPos = mul(worldPos, worldMat);
    Out.worldPos = worldPos.xyz;

    Out.pos = mul(worldPos, viewMat);
    Out.pos = mul(Out.pos, projMat);
    Out.worldNormal = normalize(mul(In.normal, (float3x3) worldMat));
    Out.worldTangent = normalize(mul(In.tangent, (float3x3) worldMat));
    
    Out.uv = In.uv;
    
    return Out;
}