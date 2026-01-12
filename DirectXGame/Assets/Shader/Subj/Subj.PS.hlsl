#include "../LightFuncs.hlsli"

struct PSInput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
    float3 normal : NORMAL0;
    float3 worldPos : WORLDPOS0;
};

struct PSOutput
{
    float4 color : SV_TARGET;
};

cbuffer PSData : register(b0)
{
    float4 baseColor;
    DirectionalLight dirLight;
    float3 cameraPos;
    float shininess;
    int textureIndex;
    int useBlinnPhong;
};

Texture2D textures[] : register(t8);
SamplerState samplerState : register(s0);

PSOutput main(PSInput input)
{
    float4 textureColor = textures[textureIndex].Sample(samplerState, input.texCoord);
    float4 objColor = baseColor * textureColor;
    
    float4 lightedColor = HalfLambert(input.normal, objColor, dirLight.color, dirLight.direction, dirLight.intensity);
    
    float specular = 0.0f;
    
    if (useBlinnPhong)
    {
    
        float3 reflectLight = reflect(dirLight.direction, normalize(input.normal));
        float RdotE = dot(reflectLight, normalize(cameraPos - input.worldPos));
        specular = pow(saturate(RdotE), shininess);
    
    }
    else
    {
        float3 halfVector = normalize(-dirLight.direction + normalize(cameraPos - input.worldPos));
        float NdotH = dot(normalize(input.normal), halfVector);
        specular = pow(saturate(NdotH), shininess);
    }
    float3 specularColor = specular * dirLight.color.rgb * dirLight.intensity * float3(1.0f, 1.0f, 1.0f);
    
    PSOutput output;
    output.color = lightedColor + float4(specularColor, 0.0f);
    return output;
}
