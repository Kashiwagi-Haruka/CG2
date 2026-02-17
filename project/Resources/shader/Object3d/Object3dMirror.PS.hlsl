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
    float padding2;
};
struct Camera
{
    float3 worldPosition;
    float padding;
    float2 screenSize;
    int fullscreenGrayscaleEnabled;
    float padding2;
};
ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<Camera> gCamera : register(b4);
Texture2D<float4> gTexture : register(t0);
Texture2D<float4> gEnvironmentTexture : register(t4);
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


PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float4 transformedUV = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
    float3 baseColor = (gMaterial.color * textureColor).rgb;

    float3 toEye = normalize(gCamera.worldPosition - input.worldPosition);
    float2 screenUV = input.position.xy / gCamera.screenSize;
    screenUV.x = 1.0f - screenUV.x;
    float3 environmentColor = gEnvironmentTexture.Sample(gSampler, saturate(screenUV)).rgb;

    float fresnel = pow(1.0f - saturate(dot(normalize(input.normal), toEye)), 5.0f);
    float mirrorStrength = saturate(gMaterial.environmentCoefficient);
    float reflectionFactor = saturate(mirrorStrength + fresnel * (1.0f - mirrorStrength));

    output.color.rgb = lerp(baseColor, environmentColor, reflectionFactor);
    output.color.a = gMaterial.color.a * textureColor.a;

    output.color.rgb = ApplyGrayscale(output.color.rgb);

    if (textureColor.a < 0.5f)
    {
        discard;
    }
    if (textureColor.a == 0.0f)
    {
        discard;
    }
    if (output.color.a == 0.0f)
    {
        discard;
    }

    return output;
}