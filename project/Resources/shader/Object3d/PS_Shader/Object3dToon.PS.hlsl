#include "../Object3d.hlsli"
struct Material
{
    float4 color;
    int enableLighting;
    float3 padding;
    float4x4 uvTransform;
    float shininess;
    float environmentCoefficient;
    int grayscaleEnabled;
    int sepiaEnabled;
    float distortionStrength;
    float distortionFalloff;
    float4 outlineColor;
    float outlineWidth;
    float3 outlinePadding;
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
    int fullscreenGrayscaleEnabled;
    int fullscreenSepiaEnabled;
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
static const float kToonShadowSoftness = 0.04f;
static const float kToonShadowStrength = 0.35f;
static const float kToonLightIntensityMax = 1.00f;
float3 ApplyGrayscale(float3 color)
{
    if (gMaterial.grayscaleEnabled == 0 && gCamera.fullscreenGrayscaleEnabled == 0)
    {
        return color;
    }
    float y = dot(color, float3(0.2125f, 0.7154f, 0.0721f));
    return float3(y, y, y);
}
float3 ApplySepia(float3 color)
{
    if (gMaterial.sepiaEnabled == 0 && gCamera.fullscreenSepiaEnabled == 0)
    {
        return color;
    }

    float3 sepia;
    sepia.r = dot(color, float3(0.393f, 0.769f, 0.189f));
    sepia.g = dot(color, float3(0.349f, 0.686f, 0.168f));
    sepia.b = dot(color, float3(0.272f, 0.534f, 0.131f));
    return saturate(sepia);
}
float ComputeToonShadowMask(float NdotL)
{
    float saturatedNdotL = saturate(NdotL);
    return smoothstep(
        kToonShadowThreshold - kToonShadowSoftness,
        kToonShadowThreshold + kToonShadowSoftness,
        saturatedNdotL);
}

PixelShaderOutput main(Object3dVertexShaderOutput input)
{
    PixelShaderOutput output;
    output.color = float4(gMaterial.outlineColor.rgb, saturate(gMaterial.outlineColor.a) * gMaterial.color.a);
    if (output.color.a <= 0.0f || gMaterial.outlineWidth <= 0.0f)
    {
        discard;
    }
    return output;
}