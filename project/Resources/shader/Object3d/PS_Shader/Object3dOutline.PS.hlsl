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
    float2 padding2;
    float4 outlineColor;
    float outlineWidth;
    float3 outlinePadding;
};

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

cbuffer MaterialBuffer : register(b0)
{
    Material gMaterial;
};

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(Object3dVertexShaderOutput input)
{
    PixelShaderOutput output;

    float4 transformedUV = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float2 uv = transformedUV.xy;
    float4 textureColor = gTexture.Sample(gSampler, uv);

    float baseAlpha = textureColor.a * gMaterial.color.a;
    if (baseAlpha <= 0.0f)
    {
        discard;
    }

    const float kMaxOutlineWidthPixels = 20.0f;
    float normalizedWidth = saturate(max(gMaterial.outlineWidth, 0.0f) / kMaxOutlineWidthPixels);

    output.color.rgb = gMaterial.outlineColor.rgb;
    output.color.a = normalizedWidth;
    return output;
}