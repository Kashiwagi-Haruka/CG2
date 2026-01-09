#include "Object3d.hlsli"
#define MAX_POINT_LIGHTS 4
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
};
struct PointLightSet
{
    PointLight lights[MAX_POINT_LIGHTS];
    int count;
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
};
struct Camera
{
    float3 worldPosition;
};
ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b3);
ConstantBuffer<Camera> gCamera : register(b4);
ConstantBuffer<PointLightSet> gPointLight : register(b5);
ConstantBuffer<SpotLight> gSpotLight : register(b6);
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
        
        float3 Lp = normalize(gPointLight.position - input.worldPosition);
        float3 H = normalize(Lp + toEye); // ハーフベクトル
        float distance = length(gPointLight.position - input.worldPosition);
        float attenuation = pow(saturate(1.0f - distance / gPointLight.radius), gPointLight.decay);

            // 拡散 (Lambert)
            float NdotL_p = saturate(dot(N, Lp));
            diffuseP += gMaterial.color.rgb * textureColor.rgb *
                  light.color.rgb * light.intensity *
                  NdotL_p * attenuation;

            // 鏡面反射 (Phong)
            float NdotH_p = saturate(dot(N, H));
            float specularPowP = pow(NdotH_p, gMaterial.shininess);
            specularP += light.color.rgb * light.intensity *
                   specularPowP * attenuation;
        }

// Spot Light
        float3 spotLightDirectionOnsurface = normalize(gSpotLight.position - input.worldPosition);
        float3 spotDirection = normalize(gSpotLight.direction);
        float specularPowS = pow(saturate(dot(N, normalize(spotLightDirectionOnsurface + toEye))), gMaterial.shininess);
        float cosAngle = dot(spotLightDirectionOnsurface, spotDirection);
        float falloffFactor = saturate((cosAngle - gSpotLight.cosAngle) / (gSpotLight.cosFalloffStart - gSpotLight.cosAngle));
        float spotDistance = length(gSpotLight.position - input.worldPosition);
        float attenuationFactor = pow(saturate(1.0f - spotDistance / max(gSpotLight.distance, 0.0001f)), gSpotLight.decay);
        
        float3 spotLightDiffuse = gMaterial.color.rgb * textureColor.rgb * gSpotLight.color.rgb * gSpotLight.intensity * attenuationFactor * falloffFactor;
        float3 spotLightSpecular = gSpotLight.color.rgb * gSpotLight.intensity * attenuationFactor * falloffFactor;
        
        output.color.rgb = diffuse + specular + diffuseP + specularP + spotLightDiffuse + spotLightSpecular;
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
