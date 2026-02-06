#include "Object3d.hlsli"
struct Material
{
    float4 color;
    int enableLighting;
    float3 padding;
    float4x4 uvTransform;
    float shininess;
    float environmentCoefficient;
    float2 padding2;
};
struct DirectionalLight
{
    float4 color;
    float3 direction;
    float intensity;
};
struct PointLight
{
    float4 color;
    float3 position;
    float intensity;
    float radius;
    float decay;
    float2 padding;
};
struct PointLightCount
{
    uint count;
    float3 padding;
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
    float cosFalloffStart;
    float2 padding;
};
struct SpotLightCount
{
    uint count;
    float3 padding;
};
struct AreaLight
{
    float4 color;
    float3 position;
    float intensity;
    float3 normal;
    float width;
    float height;
    float radius;
    float decay;
    float padding;
};
struct AreaLightCount
{
    uint count;
    float3 padding;
};
struct Camera
{
    float3 worldPosition;
    float padding;
    float2 screenSize;
    float2 padding2;
};
ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b3);
ConstantBuffer<Camera> gCamera : register(b4);
ConstantBuffer<PointLightCount> gPointLightCount : register(b5);
ConstantBuffer<SpotLightCount> gSpotLightCount : register(b6);
ConstantBuffer<AreaLightCount> gAreaLightCount : register(b7);
StructuredBuffer<SpotLight> gSpotLights : register(t2);
StructuredBuffer<PointLight> gPointLights : register(t1);
StructuredBuffer<AreaLight> gAreaLights : register(t3);
Texture2D<float4> gTexture : register(t0);
Texture2D<float4> gEnvironmentTexture : register(t4);
SamplerState gSampler : register(s0);
struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

static const float kToonShadowThreshold = 0.5f;
static const float kToonShadowStrength = 0.35f;
static const float kToonLightIntensityMax = 1.00f;

float ComputeToonShadowMask(float NdotL)
{
    return step(kToonShadowThreshold, saturate(NdotL));
}


PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    const float pi = 3.14159265f;
    float4 transformedUV = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);

    float4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
    if (gMaterial.enableLighting != 0)
    {
        float3 baseColor = gMaterial.color.rgb * textureColor.rgb;
        float3 normal = normalize(input.normal);
        float NdotL = dot(normal, -gDirectionalLight.direction);
        float toonShadowMask = ComputeToonShadowMask(NdotL);

        float directionalIntensity = min(gDirectionalLight.intensity, kToonLightIntensityMax);
        float3 litColor = baseColor * gDirectionalLight.color.rgb * directionalIntensity;
        float3 shadowColor = baseColor * kToonShadowStrength;
        float3 viewDirection = normalize(input.worldPosition - gCamera.worldPosition);
        float3 reflectedDirection = reflect(viewDirection, normalize(input.normal));
        float2 environmentUV = float2(atan2(reflectedDirection.z, reflectedDirection.x) / (2.0f * pi) + 0.5f,
            asin(reflectedDirection.y) / pi + 0.5f);
        float3 environmentColor = gEnvironmentTexture.Sample(gSampler, environmentUV).rgb;

        output.color.rgb = lerp(shadowColor, litColor, toonShadowMask);
        output.color.rgb += environmentColor * gMaterial.environmentCoefficient;
        output.color.a = gMaterial.color.a * textureColor.a;
    }
    else
    {
        output.color = gMaterial.color * textureColor;
    }
    if (textureColor.a < 0.5f)
    {
        discard;
    }
    if (textureColor.a == 0.0f)
    {
        discard;
    }
    if (output.color.a == 0.0f)
    {
        discard;
    }
    
    return output;
}