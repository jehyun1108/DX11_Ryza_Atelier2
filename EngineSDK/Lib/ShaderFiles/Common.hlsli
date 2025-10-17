#pragma pack_matrix(row_major)

// ------ Macro ---------------
#define LIGHT_DIRECTIONAL 0
#define LIGHT_POINT       1
#define LIGHT_SPOT        2

#define MAX_LIGHTS        16
#define MAX_BONES         1024

// ------ Struct -------------
struct LightData
{
    int type;
    float3 padding1;

    float4 ambient;
    float4 diffuse;
    float4 specular;

    float4 lightPos;
    float4 lightDir;

    float range;
    float spotAngle;
    float2 padding;
};

// Constant Buffers
cbuffer FrameBuffer : register(b0)
{
    matrix viewMat;
    matrix projMat;
    float4 camPos;
}

cbuffer ObjBuffer : register(b1)
{
    matrix worldMat;
    float4 color;
    matrix invWorldMat;
}

cbuffer LightBuffer : register(b2)
{
    LightData light;
    
}

cbuffer BoneBuffer : register(b3)
{
    matrix boneMat[MAX_BONES];
}