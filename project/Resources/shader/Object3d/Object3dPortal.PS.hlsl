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
};

struct TextureCamera
{
    float4x4 textureViewProjection;
    float4x4 portalCameraWorld;
    int usePortalProjection;
    float3 padding;
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
};

float2 ComputeProjectedUV(float3 worldPosition, float4x4 viewProjection, out bool valid)
{
    const float4 clip = mul(float4(worldPosition, 1.0f), viewProjection);
    if (clip.w <= 0.0001f)
    {
        valid = false;
        return float2(0.0f, 0.0f);
    }

    const float3 ndc = clip.xyz / clip.w;
    const float2 uv = float2(ndc.x * 0.5f + 0.5f, -ndc.y * 0.5f + 0.5f);
    valid = all(uv >= float2(0.0f, 0.0f)) && all(uv <= float2(1.0f, 1.0f));
    return uv;
}

PixelShaderOutput main(PortalVertexShaderOutput input)
{
    PixelShaderOutput output;

    float4 baseUV = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float4 baseColor = gTexture.Sample(gSampler, baseUV.xy) * gMaterial.color;

    if (gTextureCamera.usePortalProjection == 0)
    {
        output.color = baseColor;
        return output;
    }

    bool projectedValid = false;
    const float2 projectedTexcoord = ComputeProjectedUV(input.worldPosition, gTextureCamera.textureViewProjection, projectedValid);


    if (!projectedValid)
    {
        output.color = baseColor;
        return output;
    }

    output.color = gTextureSecondary.Sample(gSampler, projectedTexcoord) * gMaterial.color;
    return output;
}