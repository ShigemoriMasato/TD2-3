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

StructuredBuffer<SpotLight> spotLights : register(t0);

PSOutput main(VSOutput input)
{
    PSOutput output;
    
    //Gray
    output.color = float4(1.0f, 1.0f, 1.0f, 1.0f);
    float4 lightedColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    
    const int lightCount = 4;
    for (int i = 0; i < lightCount; ++i)
    {
        SpotLight light = spotLights[i];
        
        if(light.decay <= 0.0f)
            continue;
        
        float3 direction = normalize(input.worldPos - light.position);
        float cosAngle = dot(direction, normalize(light.direction));
        float falloff = saturate(-(cosAngle - light.cosAngle) / (light.falloffStart - light.cosAngle));
        
        lightedColor += HalfLambert(input.normal, output.color, light.color, light.direction, light.intensity * falloff);
    }
    
    output.color = lightedColor;
    
    return output;
}
