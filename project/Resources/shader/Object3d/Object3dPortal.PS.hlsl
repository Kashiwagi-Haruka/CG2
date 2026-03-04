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

struct PortalVertexShaderOutput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
    float3 worldPosition : POSITION0;
    float4 shadowPosition : TEXCOORD1;
    float2 projectedTexcoord : TEXCOORD2;
    float projectedValid : TEXCOORD3;
};

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

    // ポータル用レンダーテクスチャを、テクスチャカメラ行列で投影して取得。
    // 画面外はラップせず通常UVへフォールバックして、Z移動時の引き伸ばし/反復を防ぐ。
    const bool inRange = input.projectedValid > 0.5f;

    float4 projected = gTextureSecondary.Sample(gSampler, inRange ? input.projectedTexcoord : baseUV.xy);
    output.color = projected * gMaterial.color;
    return output;
}