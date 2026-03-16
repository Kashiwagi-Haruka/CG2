struct PSInput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL0;
};

cbuffer SceneConstants : register(b13)
{
    float4x4 gViewProjection;
    float4 gLightDirection;
};

float4 main(PSInput input) : SV_TARGET
{
    const float3 normal = normalize(input.normal);
    const float3 lightDir = normalize(-gLightDirection.xyz);
    const float diffuse = saturate(dot(normal, lightDir));
    const float3 baseColor = float3(0.35f, 0.20f, 0.11f);
    const float3 color = baseColor * (0.35f + diffuse * 0.65f);
    return float4(color, 1.0f);
}