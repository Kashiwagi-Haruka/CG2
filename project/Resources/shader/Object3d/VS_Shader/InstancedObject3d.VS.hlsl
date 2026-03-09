struct VSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL0;
};

StructuredBuffer<float4x4> gInstanceWorld : register(t13);

cbuffer SceneConstants : register(b13)
{
    float4x4 gViewProjection;
    float4 gLightDirection;
};

VSOutput main(VSInput input, uint instanceId : SV_InstanceID)
{
    VSOutput output;
    const float4x4 world = gInstanceWorld[instanceId];
    const float4 worldPosition = mul(float4(input.position, 1.0f), world);
    output.position = mul(worldPosition, gViewProjection);
    output.normal = normalize(mul(input.normal, (float3x3) world));
    return output;
}