struct VSInput
{
    float4 position : POSITION0;
    float2 texCoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
    float3 normal : NORMAL0;
    float3 worldPos : WORLDPOS0;
};

cbuffer MatrixBuffer : register(b0)
{
    float4x4 world;
    float4x4 vp;
};

VSOutput main(VSInput input)
{
    VSOutput output;
    output.position = mul(input.position, mul(world, vp));
    output.texCoord = input.texCoord;
    output.normal = mul(input.normal, (float3x3) world);
    output.worldPos = mul(input.position, world).xyz;
    return output;
}