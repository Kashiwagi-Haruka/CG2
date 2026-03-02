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
    float4x4 portalCameraWorld0;
    float4x4 portalCameraWorld1;
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

    float2 uv0 = input.texcoord;
    float2 uv1 = input.texcoord;
    float validMask0 = 1.0f;
    float validMask1 = 1.0f;

    if (gCamera.usePortalProjection != 0)
    {
        float4 clip0 = mul(float4(input.worldPosition, 1.0f), gCamera.textureViewProjection0);
        float4 clip1 = mul(float4(input.worldPosition, 1.0f), gCamera.textureViewProjection1);

        float2 ndc0 = clip0.xy / max(clip0.w, 1e-5f);
        float2 ndc1 = clip1.xy / max(clip1.w, 1e-5f);

        uv0 = float2(ndc0.x * 0.5f + 0.5f, -ndc0.y * 0.5f + 0.5f);
        uv1 = float2(ndc1.x * 0.5f + 0.5f, -ndc1.y * 0.5f + 0.5f);

        bool inUv0 = all(uv0 >= 0.0f.xx) && all(uv0 <= 1.0f.xx);
        bool inUv1 = all(uv1 >= 0.0f.xx) && all(uv1 <= 1.0f.xx);

        float3 cameraPosition0 = gCamera.portalCameraWorld0[3].xyz;
        float3 cameraPosition1 = gCamera.portalCameraWorld1[3].xyz;
        float3 cameraForward0 = normalize(gCamera.portalCameraWorld0[2].xyz);
        float3 cameraForward1 = normalize(gCamera.portalCameraWorld1[2].xyz);
        float front0 = dot(input.worldPosition - cameraPosition0, cameraForward0);
        float front1 = dot(input.worldPosition - cameraPosition1, cameraForward1);

        validMask0 = (clip0.w > 0.0f && front0 > 0.0f && inUv0) ? 1.0f : 0.0f;
        validMask1 = (clip1.w > 0.0f && front1 > 0.0f && inUv1) ? 1.0f : 0.0f;
    }

    float4 tex0 = gCameraTexture0.Sample(gSampler, saturate(uv0)) * validMask0;
    float4 tex1 = gCameraTexture1.Sample(gSampler, saturate(uv1)) * validMask1;

    float4 portalColor = lerp(tex0, tex1, saturate(gMaterial.environmentCoefficient));

    float2 centeredUv = input.texcoord * 2.0f - 1.0f;
    float radial = length(centeredUv);
    float rim = smoothstep(0.7f, 1.0f, radial);
    portalColor.rgb = lerp(portalColor.rgb, gMaterial.color.rgb, rim * 0.35f);

    output.color = gMaterial.color * portalColor;
    output.color.rgb = ApplyGrayscale(output.color.rgb);
    output.color.rgb = ApplySepia(output.color.rgb);

    if (output.color.a < 0.001f)
    {
        discard;
    }

    return output;
}