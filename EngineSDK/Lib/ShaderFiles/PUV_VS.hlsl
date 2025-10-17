#include "Common.hlsli"

struct VS_IN
{
    float3 pos : POSITION;
    float2 uv : TEXCOORD0;
};

struct VS_OUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out = (VS_OUT) 0;
    
    float4 worldPos = mul(float4(In.pos, 1.f), worldMat);
    float4 viewPos = mul(worldPos, viewMat);
    float4 projPos = mul(viewPos, projMat);
  
    Out.pos = projPos;
    Out.uv = In.uv;
    
    return Out;
};