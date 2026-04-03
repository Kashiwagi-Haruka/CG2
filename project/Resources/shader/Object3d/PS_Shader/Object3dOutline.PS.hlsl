#include "../Object3d.hlsli"

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
struct DirectionalLight
{
    float4 color;
    float3 direction;
    float intensity;
    int shadowEnabled;
    float3 padding;
};
struct PointLight
{
    float4 color;
    float3 position;
    float intensity;
    float radius;
    float decay;
    int shadowEnabled;
    float padding;
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
    int shadowEnabled;
    float padding;
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
    int shadowEnabled;
    float3 padding;
};
struct AreaLightCount
{
    uint count;
    float3 padding;
};
struct Camera
{
    float3 worldPosition;
    float padding;
    float2 screenSize;
    int fullscreenGrayscaleEnabled;
    int fullscreenSepiaEnabled;
    float2 padding2;
};

Texture2D<float4> gTexture : register(t0);
Texture2D<float> gDirectionalShadowMap : register(t5);
Texture2D<float> gPointShadowMap : register(t6);
Texture2D<float> gSpotShadowMap : register(t7);
Texture2D<float> gAreaShadowMap : register(t8);
SamplerState gSampler : register(s0);
cbuffer MaterialBuffer : register(b0)
{
    Material gMaterial;
};
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b3);
ConstantBuffer<Camera> gCamera : register(b4);
ConstantBuffer<PointLightCount> gPointLightCount : register(b5);
ConstantBuffer<SpotLightCount> gSpotLightCount : register(b6);
ConstantBuffer<AreaLightCount> gAreaLightCount : register(b7);
StructuredBuffer<PointLight> gPointLights : register(t1);
StructuredBuffer<SpotLight> gSpotLights : register(t2);
StructuredBuffer<AreaLight> gAreaLights : register(t3);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

float ComputeMicroShadow(float3 normal, float3 toLight, float3 toEye)
{
    float NdotL = saturate(dot(normal, toLight));
    (void) toEye;
    return pow(saturate(NdotL * 0.5f + 0.5f), 2.0f);
}

float ComputeShadowVisibility(Texture2D<float> shadowMap, float4 shadowPosition)
{
    if (shadowPosition.w <= 0.0f)
    {
        return 1.0f;
    }

    float3 shadowCoord = shadowPosition.xyz / shadowPosition.w;
    float2 shadowUV;
    shadowUV.x = shadowCoord.x * 0.5f + 0.5f;
    shadowUV.y = -shadowCoord.y * 0.5f + 0.5f;

    if (shadowUV.x < 0.0f || shadowUV.x > 1.0f || shadowUV.y < 0.0f || shadowUV.y > 1.0f)
    {
        return 1.0f;
    }

    float receiverDepth = shadowCoord.z;
    if (receiverDepth <= 0.0f || receiverDepth >= 1.0f)
    {
        return 1.0f;
    }

    uint shadowMapWidth;
    uint shadowMapHeight;
    shadowMap.GetDimensions(shadowMapWidth, shadowMapHeight);
    float2 texelSize = 1.0f / float2(shadowMapWidth, shadowMapHeight);
    const float depthBias = 0.002f;

    float visibility = 0.0f;
    [unroll]
    for (int y = -1; y <= 1; ++y)
    {
        [unroll]
        for (int x = -1; x <= 1; ++x)
        {
            float2 sampleUV = saturate(shadowUV + float2(x, y) * texelSize);
            float shadowDepth = shadowMap.Sample(gSampler, sampleUV);
            visibility += ((receiverDepth - depthBias) <= shadowDepth) ? 1.0f : 0.0f;
        }
    }

    visibility /= 9.0f;
    return lerp(0.25f, 1.0f, visibility);
}

PixelShaderOutput main(Object3dVertexShaderOutput input)
{
    PixelShaderOutput output;

    float4 transformedUV = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float2 uv = transformedUV.xy;

    float4 textureColor = gTexture.Sample(gSampler, uv);
    float3 litColor = textureColor.rgb * gMaterial.color.rgb;

    if (gMaterial.enableLighting != 0)
    {
        float3 N = normalize(input.normal);
        float3 toEye = normalize(gCamera.worldPosition - input.worldPosition);

        float directionalShadowVisibility = ComputeShadowVisibility(gDirectionalShadowMap, input.directionalShadowPosition);
        float pointShadowVisibility = ComputeShadowVisibility(gPointShadowMap, input.pointShadowPosition);
        float spotShadowVisibility = ComputeShadowVisibility(gSpotShadowMap, input.spotShadowPosition);
        float areaShadowVisibility = ComputeShadowVisibility(gAreaShadowMap, input.areaShadowPosition);

        float3 directionalLightVector = -normalize(gDirectionalLight.direction);
        float directionalHalfLambert = pow(saturate(dot(N, directionalLightVector) * 0.5f + 0.5f), 2.0f);
        float3 directionalHalfVector = normalize(directionalLightVector + toEye);
        float directionalSpecularPow = pow(saturate(dot(N, directionalHalfVector)), gMaterial.shininess);
        float directionalShadow = ComputeMicroShadow(N, directionalLightVector, toEye);
        float directionalShadowFactor = (gDirectionalLight.shadowEnabled != 0) ? directionalShadowVisibility : 1.0f;

        float3 diffuse = gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * directionalHalfLambert * gDirectionalLight.intensity * directionalShadow * directionalShadowFactor;
        float3 specular = gDirectionalLight.color.rgb * gDirectionalLight.intensity * directionalSpecularPow * directionalShadow * directionalShadowFactor;

        float3 pointDiffuse = float3(0.0f, 0.0f, 0.0f);
        float3 pointSpecular = float3(0.0f, 0.0f, 0.0f);
        for (uint pointIndex = 0; pointIndex < gPointLightCount.count; ++pointIndex)
        {
            PointLight pointLight = gPointLights[pointIndex];
            float3 pointDirection = normalize(pointLight.position - input.worldPosition);
            float3 pointHalfVector = normalize(pointDirection + toEye);
            float pointDistance = length(pointLight.position - input.worldPosition);
            float pointAttenuation = pow(saturate(1.0f - pointDistance / pointLight.radius), pointLight.decay);
            float pointShadow = ComputeMicroShadow(N, pointDirection, toEye);
            float pointShadowFactor = (pointLight.shadowEnabled != 0) ? pointShadowVisibility : 1.0f;
            float pointNdotL = saturate(dot(N, pointDirection));

            pointDiffuse += gMaterial.color.rgb * textureColor.rgb *
                pointLight.color.rgb * pointLight.intensity *
                pointNdotL * pointAttenuation * pointShadow * pointShadowFactor;

            float pointSpecularPow = pow(saturate(dot(N, pointHalfVector)), gMaterial.shininess);
            pointSpecular += pointLight.color.rgb * pointLight.intensity *
                pointSpecularPow * pointAttenuation * pointShadow * pointShadowFactor;
        }

        float3 spotDiffuse = float3(0.0f, 0.0f, 0.0f);
        float3 spotSpecular = float3(0.0f, 0.0f, 0.0f);
        for (uint spotIndex = 0; spotIndex < gSpotLightCount.count; ++spotIndex)
        {
            SpotLight spotLight = gSpotLights[spotIndex];
            float3 spotToSurface = spotLight.position - input.worldPosition;
            float3 spotDirection = normalize(spotToSurface);
            float3 spotForward = -spotDirection;
            float3 spotAxis = normalize(spotLight.direction);
            float spotCosAngle = dot(spotForward, spotAxis);
            float spotFalloff = saturate((spotCosAngle - spotLight.cosAngle) / (spotLight.cosFalloffStart - spotLight.cosAngle));
            float spotDistance = length(spotToSurface);
            float spotAttenuation = pow(saturate(1.0f - spotDistance / spotLight.distance), spotLight.decay);
            float spotShadow = ComputeMicroShadow(N, spotDirection, toEye);
            float spotShadowFactor = (spotLight.shadowEnabled != 0) ? spotShadowVisibility : 1.0f;

            float spotNdotL = saturate(dot(N, spotDirection));
            float3 spotHalfVector = normalize(spotDirection + toEye);
            float spotSpecularPow = pow(saturate(dot(N, spotHalfVector)), gMaterial.shininess);

            spotDiffuse += gMaterial.color.rgb * textureColor.rgb *
                spotLight.color.rgb * spotLight.intensity *
                spotNdotL * spotAttenuation * spotFalloff * spotShadow * spotShadowFactor;

            spotSpecular += spotLight.color.rgb * spotLight.intensity *
                spotSpecularPow * spotAttenuation * spotFalloff * spotShadow * spotShadowFactor;
        }

        float3 areaDiffuse = float3(0.0f, 0.0f, 0.0f);
        float3 areaSpecular = float3(0.0f, 0.0f, 0.0f);
        for (uint areaIndex = 0; areaIndex < gAreaLightCount.count; ++areaIndex)
        {
            AreaLight areaLight = gAreaLights[areaIndex];
            float3 areaToSurface = areaLight.position - input.worldPosition;
            float3 areaDirection = normalize(areaToSurface);
            float areaDistance = length(areaToSurface);
            float areaAttenuation = pow(saturate(1.0f - areaDistance / areaLight.radius), areaLight.decay);
            float areaFacing = saturate(dot(normalize(areaLight.normal), -areaDirection));
            float areaScale = areaLight.width * areaLight.height;
            float areaIntensity = areaLight.intensity * areaScale * areaFacing;
            float areaShadow = ComputeMicroShadow(N, areaDirection, toEye);
            float areaShadowFactor = (areaLight.shadowEnabled != 0) ? areaShadowVisibility : 1.0f;

            float areaNdotL = saturate(dot(N, areaDirection));
            float3 areaHalfVector = normalize(areaDirection + toEye);
            float areaSpecularPow = pow(saturate(dot(N, areaHalfVector)), gMaterial.shininess);

            areaDiffuse += gMaterial.color.rgb * textureColor.rgb *
                areaLight.color.rgb * areaIntensity *
                areaNdotL * areaAttenuation * areaShadow * areaShadowFactor;

            areaSpecular += areaLight.color.rgb * areaIntensity *
                areaSpecularPow * areaAttenuation * areaShadow * areaShadowFactor;
        }

        litColor = diffuse + specular + pointDiffuse + pointSpecular + spotDiffuse + spotSpecular + areaDiffuse + areaSpecular;
    }

    float3 normal = normalize(input.normal);
    float3 viewDirection = normalize(gCamera.worldPosition - input.worldPosition);

    float depthEdge = saturate((abs(ddx(input.position.z)) + abs(ddy(input.position.z))) * 180.0f);
    float normalEdge = saturate(length(fwidth(normal)) * 2.5f);
    float rim = pow(1.0f - saturate(dot(normal, viewDirection)), 2.0f);

    float outlineWidth = max(gMaterial.outlineWidth, 0.0f);
    float widthFactor = saturate(outlineWidth / 10.0f);

    float edge = max(depthEdge, normalEdge);
    edge = saturate(edge + rim * lerp(0.45f, 1.25f, widthFactor));

    float threshold = lerp(0.52f, 0.12f, widthFactor);
    float softness = lerp(0.01f, 0.14f, widthFactor);
    float aa = max(fwidth(edge), 1.0e-4f);
    float outline = smoothstep(threshold - (softness + aa), threshold + (softness + aa), edge);
    float outlineOpacity = saturate(gMaterial.outlineColor.a);
    float outlineBlend = outline * outlineOpacity;

    float3 outlined = lerp(litColor, gMaterial.outlineColor.rgb, outlineBlend);

    float baseAlpha = textureColor.a * gMaterial.color.a;
    float outlineAlpha = lerp(1.0f, outlineOpacity, outline);
    output.color = float4(outlined, baseAlpha * outlineAlpha);
    if (output.color.a <= 0.0f)
    {
        discard;
    }
    return output;
}
