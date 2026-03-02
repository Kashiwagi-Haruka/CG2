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
    if (clip.w <= 0.0001f)
    {
        return float2(-1.0f, -1.0f);
    }

    float3 ndc = clip.xyz / clip.w;
    float2 uv = float2(ndc.x * 0.5f + 0.5f, -ndc.y * 0.5f + 0.5f);

    float3 camPos = cameraWorld[3].xyz;
    float fade = saturate(1.0f - length(worldPosition - camPos) * 0.05f);
    return lerp(float2(-1.0f, -1.0f), uv, fade);
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

    float2 uv0 = ComputeProjectedUV(input.worldPosition, gTextureCamera.textureViewProjection0, gTextureCamera.portalCameraWorld0);
    float2 uv1 = ComputeProjectedUV(input.worldPosition, gTextureCamera.textureViewProjection1, gTextureCamera.portalCameraWorld1);

    float inside0 = step(0.0f, uv0.x) * step(0.0f, uv0.y) * step(uv0.x, 1.0f) * step(uv0.y, 1.0f);
    float inside1 = step(0.0f, uv1.x) * step(0.0f, uv1.y) * step(uv1.x, 1.0f) * step(uv1.y, 1.0f);

    // 投影UVが有効なときはそれを使い、無効なときはメッシュUVへフォールバックすることで、
    // カメラ角度を動かさなくても常にテクスチャカメラ映像を表示する。
    float2 sampleUV0 = lerp(baseUV.xy, saturate(uv0), inside0);
    float2 sampleUV1 = lerp(baseUV.xy, saturate(uv1), inside1);

    float4 projected0 = gTexture.Sample(gSampler, sampleUV0);
    float4 projected1 = gTextureSecondary.Sample(gSampler, sampleUV1);

    float portalSelect = saturate(inside1);
    float4 portalColor = lerp(projected0, projected1, 1);
    output.color = portalColor * gMaterial.color;
    return output;
}