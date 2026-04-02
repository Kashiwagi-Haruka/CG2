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
StructuredBuffer<PointLight> gPointLights : register(t1);
StructuredBuffer<SpotLight> gSpotLights : register(t2);
StructuredBuffer<AreaLight> gAreaLights : register(t3);
Texture2D<float4> gEnvironmentTexture : register(t4);
Texture2D<float> gDirectionalShadowMap : register(t5);
Texture2D<float> gPointShadowMap : register(t6);
Texture2D<float> gSpotShadowMap : register(t7);
Texture2D<float> gAreaShadowMap : register(t8);
SamplerState gSampler : register(s0);
ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b3);
ConstantBuffer<Camera> gCamera : register(b4);
ConstantBuffer<PointLightCount> gPointLightCount : register(b5);
ConstantBuffer<SpotLightCount> gSpotLightCount : register(b6);
ConstantBuffer<AreaLightCount> gAreaLightCount : register(b7);

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

float3 ComputeLitColor(Object3dVertexShaderOutput input, float3 textureColor)
{
    float3 toEye = normalize(gCamera.worldPosition - input.worldPosition);
    float3 N = normalize(input.normal);

    float directionalShadowVisibility = ComputeShadowVisibility(gDirectionalShadowMap, input.directionalShadowPosition);
    float pointShadowVisibility = ComputeShadowVisibility(gPointShadowMap, input.pointShadowPosition);
    float spotShadowVisibility = ComputeShadowVisibility(gSpotShadowMap, input.spotShadowPosition);
    float areaShadowVisibility = ComputeShadowVisibility(gAreaShadowMap, input.areaShadowPosition);

    float3 directionalLightVector = -normalize(gDirectionalLight.direction);
    float NdotL = dot(N, directionalLightVector);
    float cosValue = pow(saturate(NdotL * 0.5f + 0.5f), 2.0f);
    float3 halfVector = normalize(directionalLightVector + toEye);
    float NDotH = dot(N, halfVector);
    float specularPow = pow(saturate(NDotH), gMaterial.shininess);
    float directionalShadow = ComputeMicroShadow(N, directionalLightVector, toEye);
    float directionalShadowFactor = (gDirectionalLight.shadowEnabled != 0) ? directionalShadowVisibility : 1.0f;

    float3 diffuse = gMaterial.color.rgb * textureColor * gDirectionalLight.color.rgb * cosValue * gDirectionalLight.intensity * directionalShadow * directionalShadowFactor;
    float3 specular = gDirectionalLight.color.rgb * gDirectionalLight.intensity * specularPow * directionalShadow * directionalShadowFactor;

    float3 diffuseP = float3(0.0f, 0.0f, 0.0f);
    float3 specularP = float3(0.0f, 0.0f, 0.0f);
    for (uint index = 0; index < gPointLightCount.count; ++index)
    {
        PointLight pointLight = gPointLights[index];
        float3 Lp = normalize(pointLight.position - input.worldPosition);
        float3 H = normalize(Lp + toEye);
        float distance = length(pointLight.position - input.worldPosition);
        float attenuation = pow(saturate(1.0f - distance / pointLight.radius), pointLight.decay);

        float NdotLPoint = saturate(dot(N, Lp));
        float pointShadow = ComputeMicroShadow(N, Lp, toEye);
        float pointShadowFactor = (pointLight.shadowEnabled != 0) ? pointShadowVisibility : 1.0f;
        diffuseP += gMaterial.color.rgb * textureColor * pointLight.color.rgb * pointLight.intensity * NdotLPoint * attenuation * pointShadow * pointShadowFactor;

        float NdotHPoint = saturate(dot(N, H));
        float specularPowPoint = pow(NdotHPoint, gMaterial.shininess);
        specularP += pointLight.color.rgb * pointLight.intensity * specularPowPoint * attenuation * pointShadow * pointShadowFactor;
    }

    float3 spotLightDiffuse = float3(0.0f, 0.0f, 0.0f);
    float3 spotLightSpecular = float3(0.0f, 0.0f, 0.0f);
    for (uint spotIndex = 0; spotIndex < gSpotLightCount.count; ++spotIndex)
    {
        SpotLight spotLight = gSpotLights[spotIndex];
        float3 lightToSurface = spotLight.position - input.worldPosition;
        float3 lightDirection = normalize(lightToSurface);
        float3 spotDirection = normalize(spotLight.direction);
        float3 lightForward = -lightDirection;
        float cosAngle = dot(lightForward, spotDirection);
        float falloffFactor = saturate((cosAngle - spotLight.cosAngle) / (spotLight.cosFalloffStart - spotLight.cosAngle));
        float distanceToLight = length(lightToSurface);
        float attenuationFactor = pow(saturate(1.0f - distanceToLight / spotLight.distance), spotLight.decay);

        float NdotLSpot = saturate(dot(N, lightDirection));
        float spotShadow = ComputeMicroShadow(N, lightDirection, toEye);
        float3 Hs = normalize(lightDirection + toEye);
        float NdotHSpot = saturate(dot(N, Hs));
        float specularPowSpot = pow(NdotHSpot, gMaterial.shininess);
        float spotShadowFactor = (spotLight.shadowEnabled != 0) ? spotShadowVisibility : 1.0f;

        spotLightDiffuse += gMaterial.color.rgb * textureColor * spotLight.color.rgb * spotLight.intensity * NdotLSpot * attenuationFactor * falloffFactor * spotShadow * spotShadowFactor;
        spotLightSpecular += spotLight.color.rgb * spotLight.intensity * specularPowSpot * attenuationFactor * falloffFactor * spotShadow * spotShadowFactor;
    }

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

        float NdotLArea = saturate(dot(N, lightDirection));
        float areaShadow = ComputeMicroShadow(N, lightDirection, toEye);
        float3 Ha = normalize(lightDirection + toEye);
        float NdotHArea = saturate(dot(N, Ha));
        float specularPowArea = pow(NdotHArea, gMaterial.shininess);
        float intensity = areaLight.intensity * areaScale * lightFacing;
        float areaShadowFactor = (areaLight.shadowEnabled != 0) ? areaShadowVisibility : 1.0f;

        areaLightDiffuse += gMaterial.color.rgb * textureColor * areaLight.color.rgb * intensity * NdotLArea * attenuationFactor * areaShadow * areaShadowFactor;
        areaLightSpecular += areaLight.color.rgb * intensity * specularPowArea * attenuationFactor * areaShadow * areaShadowFactor;
    }

    return diffuse + specular + diffuseP + specularP + spotLightDiffuse + spotLightSpecular + areaLightDiffuse + areaLightSpecular;
}

PixelShaderOutput main(Object3dVertexShaderOutput input)
{
    PixelShaderOutput output;

    float4 transformedUV = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float2 uv = transformedUV.xy;

    float4 textureColor = gTexture.Sample(gSampler, uv);
    float3 baseColor = textureColor.rgb;
    if (gMaterial.enableLighting != 0)
    {
        baseColor = ComputeLitColor(input, textureColor.rgb);
    }
    else
    {
        baseColor = gMaterial.color.rgb * textureColor.rgb;
    }

    float3 normal = normalize(input.normal);
    float3 viewDirection = normalize(gCamera.worldPosition - input.worldPosition);

    float outlineScale = max(gMaterial.outlineWidth, 0.0f);
    float depthEdge = saturate((abs(ddx(input.position.z)) + abs(ddy(input.position.z))) * (180.0f * outlineScale));
    float normalEdge = saturate(length(fwidth(normal)) * (2.5f * outlineScale));
    float rim = pow(1.0f - saturate(dot(normal, viewDirection)), 2.0f);

    float edge = max(depthEdge, normalEdge);
    edge = saturate(edge + rim * (0.8f * outlineScale));
    float thresholdStart = saturate(0.25f / max(outlineScale, 0.001f));
    float thresholdEnd = saturate(0.6f / max(outlineScale, 0.001f));
    float outline = smoothstep(thresholdStart, thresholdEnd, edge);

    float3 outlined = lerp(baseColor, gMaterial.outlineColor.rgb, outline);
    output.color = float4(outlined, gMaterial.color.a * textureColor.a * gMaterial.outlineColor.a);

    if (output.color.a <= 0.0f)
    {
        discard;
    }
    return output;
}
