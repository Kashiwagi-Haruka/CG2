#include "Object3d.hlsli"
struct Material
{
    float4 color;
    int enableLighting;
    float3 padding;
    float4x4 uvTransform;
    float shininess;
    float environmentCoefficient;
    float2 padding2;
};
struct Camera
{
    float3 worldPosition;
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

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    const float pi = 3.14159265f;

    float4 transformedUV = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
    float3 baseColor = (gMaterial.color * textureColor).rgb;

    float3 toEye = normalize(gCamera.worldPosition - input.worldPosition);
    float3 viewDirection = normalize(input.worldPosition - gCamera.worldPosition);
    float3 reflectedDirection = reflect(viewDirection, normalize(input.normal));

    float2 environmentUV = float2(atan2(reflectedDirection.z, reflectedDirection.x) / (2.0f * pi) + 0.5f,
        asin(reflectedDirection.y) / pi + 0.5f);
    float3 environmentColor = gEnvironmentTexture.Sample(gSampler, environmentUV).rgb;

    float fresnel = pow(1.0f - saturate(dot(normalize(input.normal), toEye)), 5.0f);
    float mirrorStrength = saturate(gMaterial.environmentCoefficient);
    float reflectionFactor = saturate(mirrorStrength + fresnel * (1.0f - mirrorStrength));

    output.color.rgb = lerp(baseColor, environmentColor, reflectionFactor);
    output.color.a = gMaterial.color.a * textureColor.a;

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