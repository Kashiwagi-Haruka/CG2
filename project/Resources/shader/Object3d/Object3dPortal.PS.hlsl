#include "Object3d.hlsli"

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
};

struct Camera
{
    float3 worldPosition;
    float padding;
    float2 screenSize;
    int fullscreenGrayscaleEnabled;
    int fullscreenSepiaEnabled;
    float2 padding2;
    float4x4 textureViewProjection0;
    float4x4 textureViewProjection1;
    float4x4 portalCameraWorld0;
    float4x4 portalCameraWorld1;
    int usePortalProjection;
    float3 padding3;
};

struct TextureCamera
{
    float4x4 textureViewProjection;
    float4x4 portalCameraWorld;
    float4x4 textureWorldViewProjection;
    float3 textureWorldPosition;
    int usePortalProjection;
    int useTextureCameraForVertex;
    float2 padding;
};

ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<Camera> gCamera : register(b4);
ConstantBuffer<TextureCamera> gTextureCamera : register(b5);

Texture2D<float4> gTexture : register(t0);
Texture2D<float4> gTextureSecondary : register(t4);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

struct PortalVertexShaderOutput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
    float3 worldPosition : POSITION0;
    float4 shadowPosition : TEXCOORD1;
    float4 textureProjectedPosition : TEXCOORD2;
};

float2 ComputeTextureCameraUV(float4 projectedPosition)
{
    const float safeW = max(abs(projectedPosition.w), 0.00001f);
    const float2 ndc = projectedPosition.xy / safeW;
        return float2(ndc.x * 0.5f + 0.5f, 1.0f - (ndc.y * 0.5f + 0.5f));
}


PixelShaderOutput main(PortalVertexShaderOutput input)
{
    PixelShaderOutput output;

    // Recompute projection from world position per pixel so projected imagery stays locked
    // to world space even when the receiver object moves.
    const float4 textureProjectedPosition = mul(float4(input.worldPosition, 1.0f), gTextureCamera.textureViewProjection);
    const float2 projectedTexcoord = ComputeTextureCameraUV(textureProjectedPosition);
    if (projectedTexcoord.x < 0.0f || projectedTexcoord.x > 1.0f || projectedTexcoord.y < 0.0f || projectedTexcoord.y > 1.0f)
    {
        output.color = float4(0.05f, 0.05f, 0.1f, 1.0f);
        return output;
    }

    output.color = gTextureSecondary.Sample(gSampler, projectedTexcoord);
    return output;
}