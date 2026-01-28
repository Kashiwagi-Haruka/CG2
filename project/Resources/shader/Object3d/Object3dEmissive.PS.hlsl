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

ConstantBuffer<Material> gMaterial : register(b0);
struct Camera
{
    float3 worldPosition;
};
ConstantBuffer<Camera> gCamera : register(b4);
Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;

    float4 transformedUV = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);

    float3 baseColor = textureColor.rgb * gMaterial.color.rgb;
    float3 toEye = normalize(gCamera.worldPosition - input.worldPosition);
    float3 normal = normalize(input.normal);
    float rim = 1.0f - saturate(dot(normal, toEye));
    float softRim = smoothstep(0.0f, 0.9f, rim);
    float glow = pow(softRim, 1.4f);
    float halo = pow(softRim, 3.0f);

    output.color.rgb = baseColor * (1.4f + glow * 3.0f) + baseColor * (halo * 2.5f);
    output.color.a = textureColor.a * gMaterial.color.a;

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