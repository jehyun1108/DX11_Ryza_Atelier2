#include "Common.hlsli"

#ifdef  USE_FAST_NORMAL_XFORM 
#define USE_FAST_NORMAL_XFORM
#endif

struct VS_IN
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
    float3 tangent : TANGENT;
};

struct VS_OUT
{
    float4 pos : SV_POSITION;
    float3 worldPos : POSITION;
    float3 worldNormal : NORMAL;
    float3 worldTangent : TANGENT;
    float2 uv : TEXCOORD0;
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out = (VS_OUT) 0;
    
    float4 worldPos = mul(float4(In.pos, 1.f), worldMat);
    Out.worldPos = worldPos.xyz;
    
    float4 viewPos = mul(worldPos, viewMat);
    Out.pos = mul(viewPos, projMat);

    float3x3 nMat = (float3x3) invWorldMat;
    Out.worldNormal = normalize(mul(In.normal, nMat));
    Out.worldTangent = normalize(mul(In.tangent, nMat));

    Out.uv = In.uv;
    return Out;
}