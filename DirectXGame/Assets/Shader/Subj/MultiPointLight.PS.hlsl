#include "../LightFuncs.hlsli"

struct VSOutput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
    float3 worldPos : WORLDPOS0;
};

struct PSOutput
{
    float4 color : SV_TARGET0;
};

StructuredBuffer<PointLight> pointLights : register(t0);

PSOutput main(VSOutput input)
{
    PSOutput output;
    
    //Gray
    float4 color = float4(0.5f, 0.5f, 0.5f, 1.0f);
    
    const int lightCount = 4;
    for (int i = 0; i < 4; ++i)
    {
        PointLight light = pointLights[i];
        
        if (light.intensity <= 0.0f)
            continue;
        
        float3 direction = normalize(input.worldPos - light.posiiton);
        float intensity = PointLightAttenuation(light.posiiton, input.worldPos, light.radius, light.decay, light.intensity);
        float4 lightedColor = HalfLambert(input.normal, color, light.color, direction, intensity);
        
        output.color += lightedColor;
    }
    
    output.color.w = 1.0f;
    
    return output;
}
