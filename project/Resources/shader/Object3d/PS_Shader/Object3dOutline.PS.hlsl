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

PixelShaderOutput main(Object3dVertexShaderOutput input)
{
    PixelShaderOutput output;

    float4 transformedUV = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float2 uv = transformedUV.xy;

    // 輝度ベースではなく深度勾配を使ってエッジを検出する
    float depth = saturate(input.position.z);
    float depthDx = ddx(depth);
    float depthDy = ddy(depth);
    float edge = saturate((abs(depthDx) + abs(depthDy)) * 250.0f);
    float3 baseColor = gTexture.Sample(gSampler, uv).rgb * gMaterial.color.rgb;
    float3 outlined = (1.0f - edge) * baseColor;

    output.color = float4(outlined, gMaterial.color.a);
    if (output.color.a <= 0.0f)
    {
        discard;
    }
    return output;
}
