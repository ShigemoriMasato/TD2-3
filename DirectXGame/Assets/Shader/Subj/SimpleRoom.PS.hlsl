#include "../LightFuncs.hlsli"

struct VSOutput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
    uint instanceID : SV_InstanceID;
};

struct PSOutput
{
    float4 color : SV_TARGET0;
};

struct PSData
{
    float4 color;
};

StructuredBuffer<PSData> c : register(t0);

cbuffer DirectionalLight : register(b0)
{
    float4 color;
    float3 direction;
    float intensity;
};

PSOutput main(VSOutput input)
{
    PSOutput output;
    float4 base = c[input.instanceID].color;
    
    output.color = HalfLambert(input.normal, base, color, direction, intensity);
    
    return output;
}
