struct TransformationMatrix
{
    float4x4 WVP;
    float4x4 World;
    float4x4 WorldInverseTranspose;
};
struct DirectionalLight
{
    float4 color;
    float3 direction;
    float intensity;
};
ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b1);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b3);

struct VertexShaderInput
{
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

float4x4 MakeDirectionalLightViewProjection(float3 direction)
{
    float3 lightDirection = normalize(direction);
    float3 target = float3(0.0f, 0.0f, 0.0f);
    float3 eye = target - lightDirection * 80.0f;
    float3 up = abs(lightDirection.y) > 0.99f ? float3(0.0f, 0.0f, 1.0f) : float3(0.0f, 1.0f, 0.0f);

    float3 zAxis = normalize(target - eye);
    float3 xAxis = normalize(cross(up, zAxis));
    float3 yAxis = cross(zAxis, xAxis);

    float4x4 view = float4x4(
        xAxis.x, yAxis.x, zAxis.x, 0.0f,
        xAxis.y, yAxis.y, zAxis.y, 0.0f,
        xAxis.z, yAxis.z, zAxis.z, 0.0f,
        -dot(xAxis, eye), -dot(yAxis, eye), -dot(zAxis, eye), 1.0f);

    float width = 90.0f;
    float height = 90.0f;
    float nearClip = 1.0f;
    float farClip = 220.0f;
    float4x4 ortho = float4x4(
        2.0f / width, 0.0f, 0.0f, 0.0f,
        0.0f, 2.0f / height, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f / (farClip - nearClip), 0.0f,
        0.0f, 0.0f, -nearClip / (farClip - nearClip), 1.0f);

    return mul(view, ortho);
}

float4 main(VertexShaderInput input) : SV_POSITION
{
    float3 worldPosition = mul(input.position, gTransformationMatrix.World).xyz;
    float4x4 lightViewProjection = MakeDirectionalLightViewProjection(-gDirectionalLight.direction);
    return mul(float4(worldPosition, 1.0f), lightViewProjection);
}