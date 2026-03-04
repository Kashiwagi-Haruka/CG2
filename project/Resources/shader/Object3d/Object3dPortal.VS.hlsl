#include "Object3d.hlsli"

struct TransformationMatrix
{
    float4x4 WVP;
    float4x4 LightWVP;
    float4x4 World;
    float4x4 WorldInverseTranspose;
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

struct VertexShaderInput
{
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

struct PortalVertexShaderOutput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
    float3 worldPosition : POSITION0;
    float4 shadowPosition : TEXCOORD1;
    float2 projectedTexcoord : TEXCOORD2;
    float projectedValid : TEXCOORD3;
};

ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b1);
ConstantBuffer<TextureCamera> gTextureCamera : register(b5);

float2 ComputeProjectedUV(float3 worldPosition, float4x4 viewProjection, out float valid)
{
    const float4 clip = mul(float4(worldPosition, 1.0f), viewProjection);
    if (clip.w <= 0.0001f)
    {
        valid = 0.0f;
        return float2(-1.0f, -1.0f);
    }

    const float3 ndc = clip.xyz / clip.w;
    const float2 uv = float2(ndc.x * 0.5f + 0.5f, -ndc.y * 0.5f + 0.5f);
    valid = all(uv >= float2(0.0f, 0.0f)) && all(uv <= float2(1.0f, 1.0f)) ? 1.0f : 0.0f;
    return uv;
}

PortalVertexShaderOutput main(VertexShaderInput input)
{
    PortalVertexShaderOutput output;

    output.position = mul(input.position, gTransformationMatrix.WVP);
    output.normal = normalize(mul(input.normal, (float3x3) gTransformationMatrix.WorldInverseTranspose));
    output.texcoord = input.texcoord;
    output.worldPosition = mul(input.position, gTransformationMatrix.World).xyz;
    output.shadowPosition = mul(input.position, gTransformationMatrix.LightWVP);

    output.projectedTexcoord = ComputeProjectedUV(output.worldPosition, gTextureCamera.textureViewProjection1, output.projectedValid);
    return output;
}