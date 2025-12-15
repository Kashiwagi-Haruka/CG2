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
};
struct Camera
{
    float3 worldPosition;
};
ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b3);
ConstantBuffer<Camera> gCamera : register(b4);
ConstantBuffer<PointLight> gPointLight : register(b5);

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);
struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float4 transformedUV = mul(float4(input.texcoord, 0.0f, 1.0f),gMaterial.uvTransform);

    float4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
    if (gMaterial.enableLighting != 0)
    {
        ////half lambert
        float3 toEye = normalize(gCamera.worldPosition - input.worldPosition);
        float NdotL = dot(normalize(input.normal), -gDirectionalLight.direction);
        float cos = pow(saturate(NdotL * 0.5f + 0.5f),2.0f);
        float3 halfVector = normalize(-gDirectionalLight.direction+toEye);
        float NDotH = dot(normalize(input.normal), halfVector);
        float specularPow = pow(saturate(NDotH), gMaterial.shininess);
        
        float3 diffuse =gMaterial.color.rgb *textureColor.rgb *gDirectionalLight.color.rgb *cos *gDirectionalLight.intensity;
        float3 specular = gDirectionalLight.color.rgb * gDirectionalLight.intensity * specularPow * float3(1.0f, 1.0f, 1.0f);
        
// Point Light
        float3 N = normalize(input.normal);
        float3 Lp = normalize(gPointLight.position - input.worldPosition); // 光方向
        float3 V = normalize(gCamera.worldPosition - input.worldPosition); // 視線方向
        float3 H = normalize(Lp + V); // ハーフベクトル

        float distance = length(gPointLight.position - input.worldPosition);
        float attenuation = pow(saturate(1.0f - distance / gPointLight.radius), gPointLight.decay);

// 拡散 (Lambert)
        float NdotL_p = saturate(dot(N, Lp));
        float3 diffuseP = gMaterial.color.rgb * textureColor.rgb *
                  gPointLight.color.rgb * gPointLight.intensity *
                  NdotL_p * attenuation;

// 鏡面反射 (Phong)
        float NdotH_p = saturate(dot(N, H));
        float specularPowP = pow(NdotH_p, gMaterial.shininess);
        float3 specularP = gPointLight.color.rgb * gPointLight.intensity *
                   specularPowP * attenuation;

        output.color.rgb = diffuse+specular+diffuseP+specularP;
        output.color.a = gMaterial.color.a * textureColor.a;
    }
    else
    {
        output.color = gMaterial.color * textureColor;
    }
    if(textureColor.a < 0.5f){
        discard;
    }
    if(textureColor.a == 0.0f){
        discard;
    }
    if(output.color.a == 0.0f){
        discard;
    }
    
    return output;
}