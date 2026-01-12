
struct DirectionalLight
{
    float4 color;
    float3 direction;
    float intensity;
};

struct PointLight
{
    float4 color;
    float3 posiiton;
    float intensity;
    float radius;
    float decay;
    float2 pad;
};

struct SpotLight
{
    float4 color;
    float3 position;
    float intensity;
    float3 direction;
    float distance;
    float decay;
    float cosAngle;
    float attenuation;
    float falloffStart;
};

float PointLightAttenuation(float3 lightPos, float3 objPos, float radius, float decay, float intensity)
{
    float distance = length(lightPos - objPos);
    return pow(saturate(-distance / radius + 1.0), decay) * intensity;
}

float PointLightAttenuationSimply(float3 lightPos, float3 objPos, float intensity)
{
    float distance = length(objPos - lightPos);
    return (1.0f - (distance / 10.0f - intensity)) / 2.0f;
}

float4 HalfLambert(float3 normal, float4 color, float4 lightColor, float3 direction, float intensity)
{
    float NdotL = dot(normalize(normal), -direction);
    float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
    float4 outputColor = color * lightColor * cos * intensity;
    outputColor.w = color.w;
    return outputColor;
}

float4 LambertReflectance(float3 normal, float4 color, float4 lightColor, float3 direction, float intensity)
{
    float NdotL = dot(normalize(normal), -direction);
    float cos = saturate(dot(normalize(normal), -direction));
    float4 outputColor = color * lightColor * cos * intensity;
    outputColor.w = color.w;
    return outputColor;
}

