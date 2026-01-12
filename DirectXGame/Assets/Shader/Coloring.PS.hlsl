
struct VSOutput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

struct PSOutput
{
    float4 color : SV_TARGET0;
};

cbuffer PSData : register(b0)
{
    float4 baseColor;
};

PSOutput main(VSOutput input)
{
    PSOutput output;
    output.color = baseColor;
    return output;
}
