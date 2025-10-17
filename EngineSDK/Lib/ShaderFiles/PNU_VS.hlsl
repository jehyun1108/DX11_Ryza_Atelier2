#include "Common.hlsli"

struct VS_IN
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

struct VS_OUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 worldNormal : NORMAL;
    float3 worldPos : TEXCOORD1;
};

VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out = (VS_OUT) 0;
    
    Out.worldPos = mul(float4(In.pos, 1.f), worldMat).xyz;
    float4 viewPos = mul(float4(Out.worldPos, 1.f), viewMat);
    float4 projPos = mul(viewPos, projMat);
    Out.pos = projPos;
    
    Out.worldNormal = mul(float4(In.normal, 0.f), worldMat).xyz;
    Out.uv = In.uv;
    return Out;
}