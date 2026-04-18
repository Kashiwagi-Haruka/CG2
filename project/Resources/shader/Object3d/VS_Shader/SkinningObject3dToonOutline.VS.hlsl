#include "../Object3d.hlsli"

struct TransformationMatrix
{
    float4x4 WVP;
    float4x4 DirectionalLightWVP;
    float4x4 PointLightWVP;
    float4x4 SpotLightWVP;
    float4x4 AreaLightWVP;
    float4x4 World;
    float4x4 WorldInverseTranspose;
};

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

ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b1);

struct VertexShaderInput
{
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

Object3dVertexShaderOutput main(VertexShaderInput input)
{
    Object3dVertexShaderOutput output;

    float3 normal = normalize(input.normal);
    const float outlineScale = 0.01f;
    float4 outlinedPosition = input.position;
    outlinedPosition.xyz += normal * gMaterial.outlineWidth * outlineScale;

    output.position = mul(outlinedPosition, gTransformationMatrix.WVP);
    output.normal = normalize(mul(normal, (float3x3) gTransformationMatrix.WorldInverseTranspose));
    output.texcoord = input.texcoord;
    output.worldPosition = mul(outlinedPosition, gTransformationMatrix.World).xyz;
    output.directionalShadowPosition = mul(outlinedPosition, gTransformationMatrix.DirectionalLightWVP);
    output.pointShadowPosition = mul(outlinedPosition, gTransformationMatrix.PointLightWVP);
    output.spotShadowPosition = mul(outlinedPosition, gTransformationMatrix.SpotLightWVP);
    output.areaShadowPosition = mul(outlinedPosition, gTransformationMatrix.AreaLightWVP);
    return output;
}