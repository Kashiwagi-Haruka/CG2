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
    int usePortalProjection;
    float3 padding3;
};

ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<Camera> gCamera : register(b4);
Texture2D<float4> gCameraTexture0 : register(t0);
Texture2D<float4> gCameraTexture1 : register(t4);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

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

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;

    float2 uv0;
    float2 uv1;
    if (gCamera.usePortalProjection != 0)
    {
        float4 clip0 = mul(float4(input.worldPosition, 1.0f), gCamera.textureViewProjection0);
        float4 clip1 = mul(float4(input.worldPosition, 1.0f), gCamera.textureViewProjection1);
        float2 ndc0 = clip0.xy / max(clip0.w, 0.0001f);
        float2 ndc1 = clip1.xy / max(clip1.w, 0.0001f);
        uv0 = float2(ndc0.x * 0.5f + 0.5f, -ndc0.y * 0.5f + 0.5f);
        uv1 = float2(ndc1.x * 0.5f + 0.5f, -ndc1.y * 0.5f + 0.5f);
    }
    else
    {
        float4 transformedUV = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
        uv0 = transformedUV.xy;
        uv1 = transformedUV.xy;
    }

    float4 cameraTexture0Color = gCameraTexture0.Sample(gSampler, saturate(uv0));
    float4 cameraTexture1Color = gCameraTexture1.Sample(gSampler, saturate(uv1));
    float cameraBlend = saturate(gMaterial.environmentCoefficient);
    float4 textureColor = lerp(cameraTexture0Color, cameraTexture1Color, cameraBlend);

    output.color = gMaterial.color * textureColor;
    output.color.rgb = ApplyGrayscale(output.color.rgb);
    output.color.rgb = ApplySepia(output.color.rgb);

    if (textureColor.a < 0.001f || output.color.a < 0.001f)
    {
        discard;
    }

    return output;
}