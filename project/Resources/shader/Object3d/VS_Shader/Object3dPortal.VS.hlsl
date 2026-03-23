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

struct TextureCamera
{
    float4x4 textureViewProjection;
    float4x4 portalCameraWorld;
    float4x4 textureWorldViewProjection;
    float3 textureWorldPosition;
    int usePortalProjection;
    float3 padding;
};

struct VertexShaderInput
{
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

struct Object3dPortalVertexShaderOutput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
    float3 worldPosition : TEXCOORD3;
    float4 shadowPosition : TEXCOORD1;
    float4 textureProjectedPosition : TEXCOORD2;
};

ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b1);
ConstantBuffer<TextureCamera> gTextureCamera : register(b5);

Object3dPortalVertexShaderOutput main(VertexShaderInput input)
{
    Object3dPortalVertexShaderOutput output;

    output.position = mul(input.position, gTransformationMatrix.WVP);
    output.normal = normalize(mul(input.normal, (float3x3) gTransformationMatrix.WorldInverseTranspose));
    output.texcoord = input.texcoord;
    output.worldPosition = mul(input.position, gTransformationMatrix.World).xyz;
    output.shadowPosition = mul(input.position, gTransformationMatrix.DirectionalLightWVP);
    output.textureProjectedPosition = mul(input.position, mul(gTransformationMatrix.World, gTextureCamera.textureViewProjection));
    return output;
}