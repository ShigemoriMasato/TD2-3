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
    float4x4 worldInv;
}

VSOutput main(VSInput input)
{
    VSOutput output;
    float4x4 wvp = mul(world, vp);
    output.position = mul(input.position, wvp);
    output.texCoord = input.texCoord;
    output.normal = mul(input.normal, (float3x3) worldInv);
    output.worldPos = mul(input.position, world).xyz;
    return output;
}
