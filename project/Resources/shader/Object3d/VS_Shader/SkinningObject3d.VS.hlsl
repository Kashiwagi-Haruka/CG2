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
    output.position = mul(input.position, gTransformationMatrix.WVP);
    output.normal = normalize(mul(input.normal, (float3x3) gTransformationMatrix.WorldInverseTranspose));
    output.texcoord = input.texcoord;
    output.worldPosition = mul(input.position, gTransformationMatrix.World).xyz;
    output.directionalShadowPosition = mul(input.position, gTransformationMatrix.DirectionalLightWVP);
    output.pointShadowPosition = mul(input.position, gTransformationMatrix.PointLightWVP);
    output.spotShadowPosition = mul(input.position, gTransformationMatrix.SpotLightWVP);
    output.areaShadowPosition = mul(input.position, gTransformationMatrix.AreaLightWVP);
    return output;
}