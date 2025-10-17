#include "Common.hlsli"

struct PS_IN
{
    float4 worldPos : SV_POSITION;
    float4 color : COLOR0;
};

float4 PS_MAIN(PS_IN In) : SV_TARGET
{
    return In.color;
}