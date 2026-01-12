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
    float4 color = float4(0.5f, 0.5f, 0.5f, 1.0f);
    
    const int lightCount = 4;
    for (int i = 0; i < 4; ++i)
    {
        SpotLight light = spotLights[i];
        
        if(light.decay <= 0.0f)
            continue;
        
        float3 direction = normalize(input.worldPos - light.position);
        float cosAngle = dot(direction, normalize(light.direction));
        float falloff = saturate((cosAngle - light.cosAngle) / (light.falloffStart - light.cosAngle));
        
        output.color += HalfLambert(input.normal, color, light.color, light.direction, light.intensity * falloff);
    }
    
    output.color.w = 1.0f;
    
    return output;
}
