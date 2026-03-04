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
    float4x4 textureViewProjection0;
    float4x4 textureViewProjection1;
    float4x4 portalCameraWorld0;
    float4x4 portalCameraWorld1;
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

float2 ComputeProjectedUV(float3 worldPosition, float4x4 viewProjection)
{
    const float4 clip = mul(float4(worldPosition, 1.0f), viewProjection);
    if (clip.w <= 0.0001f)
    {
        return float2(-1.0f, -1.0f);
    }

    const float3 ndc = clip.xyz / clip.w;
    return float2(ndc.x * 0.5f + 0.5f, -ndc.y * 0.5f + 0.5f);
}

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;

    float4 baseUV = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float4 baseColor = gTexture.Sample(gSampler, baseUV.xy) * gMaterial.color;

    if (gTextureCamera.usePortalProjection == 0)
    {
        output.color = baseColor;
        return output;
    }

    // ポータル用レンダーテクスチャを、テクスチャカメラ行列で投影して取得。
    // 画面外はラップせず通常UVへフォールバックして、Z移動時の引き伸ばし/反復を防ぐ。
    const float2 projectedUV = ComputeProjectedUV(input.worldPosition, gTextureCamera.textureViewProjection1);
    const bool inRange = all(projectedUV >= float2(0.0f, 0.0f)) && all(projectedUV <= float2(1.0f, 1.0f));

    float4 projected = gTextureSecondary.Sample(gSampler, inRange ? projectedUV : baseUV.xy);
    output.color = projected * gMaterial.color;
    return output;
}