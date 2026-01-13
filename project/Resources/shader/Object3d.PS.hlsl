#include "Object3d.hlsli"
struct Material
{
    float4 color;
    int enableLighting;
    float4x4 uvTransform;
    float shininess;
};
struct DirectionalLight
{
    float4 color;
    float3 direction;
    float intensity;
};
struct PointLight
{
    float4 color;
    float3 position;
    float intensity;
    float radius;
    float decay;
    float2 padding;
};
struct PointLightCount
{
    uint count;
    float3 padding;
};
struct SpotLight
{
    float4 color;
    float3 position;
    float intensity;
    float3 direction;
    float distance;
    float decay;
    float cosAngle;
    float cosFalloffStart;
    float2 padding;
};
struct SpotLightCount
{
    uint count;
    float3 padding;
};
struct AreaLight
{
    float4 color;
    float3 position;
    float intensity;
    float3 normal;
    float width;
    float height;
    float radius;
    float decay;
    float padding;
};
struct AreaLightCount
{
    uint count;
    float3 padding;
};
struct Camera
{
    float3 worldPosition;
};
ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b3);
ConstantBuffer<Camera> gCamera : register(b4);
ConstantBuffer<PointLightCount> gPointLightCount : register(b5);
ConstantBuffer<SpotLightCount> gSpotLightCount : register(b6);
ConstantBuffer<AreaLightCount> gAreaLightCount : register(b7);
StructuredBuffer<SpotLight> gSpotLights : register(t2);
StructuredBuffer<PointLight> gPointLights : register(t1);
StructuredBuffer<AreaLight> gAreaLights : register(t3);
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
    if (gMaterial.enableLighting != 0)
    {
        ////half lambert
        float3 toEye = normalize(gCamera.worldPosition - input.worldPosition);
        float NdotL = dot(normalize(input.normal), -gDirectionalLight.direction);
        float cos = pow(saturate(NdotL * 0.5f + 0.5f), 2.0f);
        float3 halfVector = normalize(-gDirectionalLight.direction + toEye);
        float NDotH = dot(normalize(input.normal), halfVector);
        float specularPow = pow(saturate(NDotH), gMaterial.shininess);
        
        float3 diffuse = gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
        float3 specular = gDirectionalLight.color.rgb * gDirectionalLight.intensity * specularPow * float3(1.0f, 1.0f, 1.0f);
        
// Point Light
        float3 N = normalize(input.normal);
        
        float3 diffuseP = float3(0.0f, 0.0f, 0.0f);
        float3 specularP = float3(0.0f, 0.0f, 0.0f);
        for (uint index = 0; index < gPointLightCount.count; ++index)
        {
            PointLight pointLight = gPointLights[index];
            float3 Lp = normalize(pointLight.position - input.worldPosition);
            float3 H = normalize(Lp + toEye); // ハーフベクトル
            float distance = length(pointLight.position - input.worldPosition);
            float attenuation = pow(saturate(1.0f - distance / pointLight.radius), pointLight.decay);

// 拡散 (Lambert)
            float NdotL_p = saturate(dot(N, Lp));
            diffuseP += gMaterial.color.rgb * textureColor.rgb *
                  pointLight.color.rgb * pointLight.intensity *
                  NdotL_p * attenuation;

// 鏡面反射 (Phong)
            float NdotH_p = saturate(dot(N, H));
            float specularPowP = pow(NdotH_p, gMaterial.shininess);
            specularP += pointLight.color.rgb * pointLight.intensity *
                   specularPowP * attenuation;
        }

// Spot Light
        float3 spotLightDiffuse = float3(0.0f, 0.0f, 0.0f);
        float3 spotLightSpecular = float3(0.0f, 0.0f, 0.0f);
        for (uint spotIndex = 0; spotIndex < gSpotLightCount.count; ++spotIndex)
        {
            SpotLight spotLight = gSpotLights[spotIndex];
            float3 lightToSurface = spotLight.position - input.worldPosition;
            float3 lightDirection = normalize(lightToSurface);
            float3 spotDirection = normalize(spotLight.direction);
            float cosAngle = dot(lightDirection, spotDirection);
            float falloffFactor = saturate((cosAngle - spotLight.cosAngle) / (spotLight.cosFalloffStart - spotLight.cosAngle));
            float distanceToLight = length(lightToSurface);
            float attenuationFactor = pow(saturate(1.0f - distanceToLight / spotLight.distance), spotLight.decay);

            float NdotL_s = saturate(dot(N, lightDirection));
            float3 Hs = normalize(lightDirection + toEye);
            float NdotH_s = saturate(dot(N, Hs));
            float specularPowS = pow(NdotH_s, gMaterial.shininess);

            spotLightDiffuse += gMaterial.color.rgb * textureColor.rgb *
                spotLight.color.rgb * spotLight.intensity *
                NdotL_s * attenuationFactor * falloffFactor;
            spotLightSpecular += spotLight.color.rgb * spotLight.intensity *
                specularPowS * attenuationFactor * falloffFactor;
        }
        // Area Light
        float3 areaLightDiffuse = float3(0.0f, 0.0f, 0.0f);
        float3 areaLightSpecular = float3(0.0f, 0.0f, 0.0f);
        for (uint areaIndex = 0; areaIndex < gAreaLightCount.count; ++areaIndex)
        {
            AreaLight areaLight = gAreaLights[areaIndex];
            float3 lightToSurface = areaLight.position - input.worldPosition;
            float3 lightDirection = normalize(lightToSurface);
            float distanceToLight = length(lightToSurface);
            float attenuationFactor = pow(saturate(1.0f - distanceToLight / areaLight.radius), areaLight.decay);
            float lightFacing = saturate(dot(normalize(areaLight.normal), -lightDirection));
            float areaScale = areaLight.width * areaLight.height;

            float NdotL_a = saturate(dot(N, lightDirection));
            float3 Ha = normalize(lightDirection + toEye);
            float NdotH_a = saturate(dot(N, Ha));
            float specularPowA = pow(NdotH_a, gMaterial.shininess);
            float intensity = areaLight.intensity * areaScale * lightFacing;

            areaLightDiffuse += gMaterial.color.rgb * textureColor.rgb *
                areaLight.color.rgb * intensity *
                NdotL_a * attenuationFactor;
            areaLightSpecular += areaLight.color.rgb * intensity *
                specularPowA * attenuationFactor;
        }
        output.color.rgb = diffuse + specular + diffuseP + specularP + spotLightDiffuse + spotLightSpecular + areaLightDiffuse + areaLightSpecular;
        output.color.a = gMaterial.color.a * textureColor.a;
    }
    else
    {
        output.color = gMaterial.color * textureColor;
    }
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