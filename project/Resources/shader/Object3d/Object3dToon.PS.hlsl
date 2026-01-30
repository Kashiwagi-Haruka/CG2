#include "Object3d.hlsli"
struct Material
{
    float4 color;
    int enableLighting;
    float3 padding;
    float4x4 uvTransform;
    float shininess;
    float environmentCoefficient;
    float2 padding2;
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
Texture2D<float4> gEnvironmentTexture : register(t4);
SamplerState gSampler : register(s0);
struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

static const float kToonShadowThreshold = 0.32f; // ← 影を狭める
static const float kToonShadowStrength = 0.52f; // ← 影をやや濃く
static const float kToonSpecularThreshold = 0.42f;
static const float kToonSpecularSoftness = 0.22f;
static const float kToonSpecularStrength = 0.12f; // 主張させない
static const float kToonRimThreshold = 0.28f;
static const float kToonRimSoftness = 0.20f;
static const float kToonRimPower = 1.6f;
static const float kToonRimStrength = 0.10f;
static const float kToonEdgeSoftness = 0.10f; // ← 境界をかなり柔らかく
static const float kToonLightWrap = 0.45f; // ← 回り込み強め
static const float kToonAmbientStrength = 0.06f;
static const float kToonColorPreserveStrength = 0.50f;
static const float kToonLightIntensityMax = 0.65f;

float ComputeToonStep(float NdotL)
{
    float wrappedNdotL = saturate((NdotL + kToonLightWrap) / (1.0f + kToonLightWrap));
    float shadowStep = smoothstep(kToonShadowThreshold - kToonEdgeSoftness, kToonShadowThreshold + kToonEdgeSoftness, wrappedNdotL);
    return lerp(kToonShadowStrength, 1.0f, shadowStep);
}

float ComputeToonSpecular(float specularPow)
{
    return smoothstep(kToonSpecularThreshold - kToonSpecularSoftness, kToonSpecularThreshold + kToonSpecularSoftness, specularPow);
}

float ComputeToonRim(float rim)
{
    return smoothstep(kToonRimThreshold - kToonRimSoftness, kToonRimThreshold + kToonRimSoftness, rim);
}

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    const float pi = 3.14159265f;
    float4 transformedUV = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);

    float4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
    if (gMaterial.enableLighting != 0)
    {
        ////half lambert
        float3 toEye = normalize(gCamera.worldPosition - input.worldPosition);
        float NdotL = dot(normalize(input.normal), -gDirectionalLight.direction);
        float3 halfVector = normalize(-gDirectionalLight.direction + toEye);
        float NDotH = dot(normalize(input.normal), halfVector);
        float specularPow = pow(saturate(NDotH), gMaterial.shininess);
        
        float toonDiffuseStep = ComputeToonStep(NdotL);
        float toonSpecularStep = ComputeToonSpecular(specularPow);
        float rim = pow(1.0f - saturate(dot(normalize(input.normal), toEye)), kToonRimPower);
        float rimStep = ComputeToonRim(rim);

        float3 baseColor = gMaterial.color.rgb * textureColor.rgb;
        float3 shadowTint = baseColor * float3(0.65f, 0.68f, 0.75f);
        float directionalIntensity = min(gDirectionalLight.intensity, kToonLightIntensityMax);
        float3 litDiffuse = baseColor * gDirectionalLight.color.rgb * directionalIntensity;
        float3 diffuse = lerp(shadowTint, litDiffuse, toonDiffuseStep);
        float3 specular = gDirectionalLight.color.rgb * directionalIntensity * toonSpecularStep * kToonSpecularStrength;
        float3 rimLight = gDirectionalLight.color.rgb * directionalIntensity * rimStep * kToonRimStrength;
        float3 ambient = baseColor * kToonAmbientStrength;
        
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
            float toonDiffuseStepP = ComputeToonStep(NdotL_p);
            float pointIntensity = min(pointLight.intensity, kToonLightIntensityMax);
            float3 litDiffuseP = baseColor * pointLight.color.rgb * pointIntensity * attenuation;
            diffuseP += litDiffuseP * toonDiffuseStepP;

// 鏡面反射 (Phong)
            float NdotH_p = saturate(dot(N, H));
            float specularPowP = pow(NdotH_p, gMaterial.shininess);
            float toonSpecularStepP = ComputeToonSpecular(specularPowP);
            specularP += pointLight.color.rgb * pointIntensity *
                   toonSpecularStepP * kToonSpecularStrength * attenuation;
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
            float toonDiffuseStepS = ComputeToonStep(NdotL_s);
            float toonSpecularStepS = ComputeToonSpecular(specularPowS);

            float spotIntensity = min(spotLight.intensity, kToonLightIntensityMax);
            float3 litDiffuseS = baseColor * spotLight.color.rgb * spotIntensity *
                attenuationFactor * falloffFactor;
            spotLightDiffuse += litDiffuseS * toonDiffuseStepS;
            spotLightSpecular += spotLight.color.rgb * spotIntensity *
                toonSpecularStepS * kToonSpecularStrength * attenuationFactor * falloffFactor;
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
            float toonDiffuseStepA = ComputeToonStep(NdotL_a);
            float toonSpecularStepA = ComputeToonSpecular(specularPowA);
            float areaIntensity = min(areaLight.intensity, kToonLightIntensityMax);
            float intensity = areaIntensity * areaScale * lightFacing;

            float3 litDiffuseA = baseColor * areaLight.color.rgb * intensity *
                attenuationFactor;
            areaLightDiffuse += litDiffuseA * toonDiffuseStepA;
            areaLightSpecular += areaLight.color.rgb * intensity *
                toonSpecularStepA * kToonSpecularStrength * attenuationFactor;
        }
        float3 viewDirection = normalize(input.worldPosition - gCamera.worldPosition);
        float3 reflectedDirection = reflect(viewDirection, normalize(input.normal));
        float2 environmentUV = float2(atan2(reflectedDirection.z, reflectedDirection.x) / (2.0f * pi) + 0.5f,
            asin(reflectedDirection.y) / pi + 0.5f);
        float3 environmentColor = gEnvironmentTexture.Sample(gSampler, environmentUV).rgb;

        float otherLightMask = toonDiffuseStep;
        float3 otherLights = diffuseP + specularP + spotLightDiffuse + spotLightSpecular + areaLightDiffuse + areaLightSpecular;
        float3 lightingSum = diffuse + specular + rimLight + ambient + (otherLights * otherLightMask);
        output.color.rgb = lerp(baseColor, lightingSum, kToonColorPreserveStrength);
        output.color.rgb += environmentColor * gMaterial.environmentCoefficient;
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