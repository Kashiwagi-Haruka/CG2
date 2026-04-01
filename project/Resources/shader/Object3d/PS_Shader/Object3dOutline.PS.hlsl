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

float Luminance(float3 rgb)
{
    return dot(rgb, float3(0.2125f, 0.7154f, 0.0721f));
}

PixelShaderOutput main(Object3dVertexShaderOutput input)
{
    PixelShaderOutput output;

    float4 transformedUV = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float2 uv = transformedUV.xy;

    float texWidth = 1.0f;
    float texHeight = 1.0f;
    float mipLevels = 1.0f;
    gTexture.GetDimensions(0, texWidth, texHeight, mipLevels);
    float2 texelSize = 1.0f / max(float2(texWidth, texHeight), float2(1.0f, 1.0f));

    static const float kPrewittHorizontalKernel[3][3] =
    {
        { -1.0f / 6.0f, 0.0f, 1.0f / 6.0f },
        { -1.0f / 6.0f, 0.0f, 1.0f / 6.0f },
        { -1.0f / 6.0f, 0.0f, 1.0f / 6.0f }
    };
    static const float kPrewittVerticalKernel[3][3] =
    {
        { -1.0f / 6.0f, -1.0f / 6.0f, -1.0f / 6.0f },
        { 0.0f, 0.0f, 0.0f },
        { 1.0f / 6.0f, 1.0f / 6.0f, 1.0f / 6.0f }
    };
    static const float2 kIndex3x3[3][3] =
    {
        { float2(-1.0f, -1.0f), float2(0.0f, -1.0f), float2(1.0f, -1.0f) },
        { float2(-1.0f, 0.0f), float2(0.0f, 0.0f), float2(1.0f, 0.0f) },
        { float2(-1.0f, 1.0f), float2(0.0f, 1.0f), float2(1.0f, 1.0f) }
    };

    float2 difference = float2(0.0f, 0.0f);
    [unroll]
    for (int y = 0; y < 3; ++y)
    {
        [unroll]
        for (int x = 0; x < 3; ++x)
        {
            float2 sampleUv = saturate(uv + kIndex3x3[y][x] * texelSize);
            float3 sampleColor = gTexture.Sample(gSampler, sampleUv).rgb;
            float luminance = Luminance(sampleColor);
            difference.x += luminance * kPrewittHorizontalKernel[y][x];
            difference.y += luminance * kPrewittVerticalKernel[y][x];
        }
    }

    float edge = saturate(length(difference) * 6.0f);
    float3 baseColor = gTexture.Sample(gSampler, uv).rgb * gMaterial.color.rgb;
    float3 outlined = (1.0f - edge) * baseColor;

    output.color = float4(outlined, gMaterial.color.a);
    if (output.color.a <= 0.0f)
    {
        discard;
    }
    return output;
}
