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
    uint instanceID : SV_InstanceID;
};

struct MatrixBuffer
{
    float4x4 world;
    float4x4 vp;
};

StructuredBuffer<MatrixBuffer> matrices : register(t0);

VSOutput main(VSInput input, uint id : SV_InstanceID)
{
    MatrixBuffer mat = matrices[id];
    VSOutput output;
    output.position = mul(input.position, mul(mat.world, mat.vp));
    output.texCoord = input.texCoord;
    output.normal = mul(input.normal, (float3x3) mat.world);
    output.instanceID = id;
    return output;
}
