cbuffer TransformMatrix : register(b0)
{
    matrix world;
    matrix vp;
};

struct Well
{
    float4x4 skeletonSpaceMatrix;
    float4x4 skeletonSpaceInverseTransposeMatrix;
};
StructuredBuffer<Well> matrixPallete : register(t0);

struct VSInput
{
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
    float4 weight : WEIGHT0;
    int4 boneIndex : INDEX0;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
    float3 worldPos : WORLDPOS0;
};

VSOutput Skinning(VSInput input)
{
    if (input.weight.x + input.weight.y + input.weight.z + input.weight.w == 0.0)
    {
        VSOutput output;
        output.position = input.position;
        output.texcoord = input.texcoord;
        output.normal = input.normal;
        return output;
    };
    
    float4 skinnedPosition = float4(0.0, 0.0, 0.0, 0.0);
    float3 skinnedNormal = float3(0.0, 0.0, 0.0);
    skinnedPosition = mul(input.position, matrixPallete[input.boneIndex.x].skeletonSpaceMatrix) * input.weight.x;
    skinnedPosition += mul(input.position, matrixPallete[input.boneIndex.y].skeletonSpaceMatrix) * input.weight.y;
    skinnedPosition += mul(input.position, matrixPallete[input.boneIndex.z].skeletonSpaceMatrix) * input.weight.z;
    skinnedPosition += mul(input.position, matrixPallete[input.boneIndex.w].skeletonSpaceMatrix) * input.weight.w;

    skinnedNormal = normalize(mul(input.normal, (float3x3) matrixPallete[input.boneIndex.x].skeletonSpaceInverseTransposeMatrix)) * input.weight.x;
    skinnedNormal += normalize(mul(input.normal, (float3x3) matrixPallete[input.boneIndex.y].skeletonSpaceInverseTransposeMatrix)) * input.weight.y;
    skinnedNormal += normalize(mul(input.normal, (float3x3) matrixPallete[input.boneIndex.z].skeletonSpaceInverseTransposeMatrix)) * input.weight.z;
    skinnedNormal += normalize(mul(input.normal, (float3x3) matrixPallete[input.boneIndex.w].skeletonSpaceInverseTransposeMatrix)) * input.weight.w;
    
    VSOutput output;
    output.position = skinnedPosition;
    output.texcoord = input.texcoord;
    output.normal = skinnedNormal;
    return output;
};

VSOutput main(VSInput input)
{
    VSOutput output;
    
    VSOutput skinned = Skinning(input);
    
    output.position = mul(skinned.position, mul(world, vp));
    output.texcoord = skinned.texcoord;
    output.normal = normalize(mul(skinned.normal, (float3x3) world));
    output.worldPos = mul(skinned.position, world).xyz;
    return output;
}
