
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

Texture2D gTexture[] : register(t8);
SamplerState gSampler : register(s0);

PSOutput main(VSOutput input)
{
    PSOutput output;
    //UVCheckerが100%入ってるとこ -> 1
    output.color = gTexture[1].Sample(gSampler, input.texcoord);
    return output;
}
