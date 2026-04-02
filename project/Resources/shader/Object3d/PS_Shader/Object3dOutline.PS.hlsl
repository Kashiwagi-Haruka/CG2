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
struct Camera
{
    float3 worldPosition;
    float padding;
    float2 screenSize;
    int fullscreenGrayscaleEnabled;
    int fullscreenSepiaEnabled;
    float2 padding2;
};

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);
cbuffer MaterialBuffer : register(b0)
{
    Material gMaterial;
};
ConstantBuffer<Camera> gCamera : register(b4);

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
    float3 baseColor = textureColor.rgb * gMaterial.color.rgb;
    float3 normal = normalize(input.normal);
    float3 viewDirection = normalize(gCamera.worldPosition - input.worldPosition);

    float depthEdge = saturate((abs(ddx(input.position.z)) + abs(ddy(input.position.z))) * 180.0f);
    float normalEdge = saturate(length(fwidth(normal)) * 2.5f);
    float rim = pow(1.0f - saturate(dot(normal, viewDirection)), 2.0f);

    float edge = max(depthEdge, normalEdge);
    edge = saturate(edge + rim * 0.8f);
    float outline = step(0.35f, edge);
    float3 outlined = lerp(baseColor, gMaterial.outlineColor.rgb, outline);

    output.color = float4(outlined, textureColor.a * gMaterial.color.a);
    if (output.color.a <= 0.0f)
    {
        discard;
    }
    return output;
}