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

float2 ComputeProjectedUV(float3 worldPosition, float4x4 viewProjection, float4x4 cameraWorld)
{
    float4 clip = mul(float4(worldPosition, 1.0f), viewProjection);
    const float safeW = abs(clip.w);
    if (safeW <= 0.0001f)
    {
        return float2(-1.0f, -1.0f);
    }

    float3 ndc = clip.xyz / safeW;
    float2 uv = float2(ndc.x * 0.5f + 0.5f, -ndc.y * 0.5f + 0.5f);

    return uv;
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
    float2 uv1 = ComputeProjectedUV(input.worldPosition, gTextureCamera.textureViewProjection1, gTextureCamera.portalCameraWorld1);
    float inside1 = step(0.0f, uv1.x) * step(0.0f, uv1.y) * step(uv1.x, 1.0f) * step(uv1.y, 1.0f);

    // ポータル描画は投影テクスチャのみを使い、メインカメラ/メッシュUVへのフォールバックを行わない。
    // これで「メイン描画がポータルテクスチャへ混ざる」見え方を防ぐ。
    if (inside1 < 0.5f)
    {
        discard;
    }

    float4 projected1 = gTextureSecondary.Sample(gSampler, uv1);
    output.color = projected1 * gMaterial.color;
    return output;
}