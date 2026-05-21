#include "../Object3d.hlsli"

struct Material
{
    float4 color;
    int enableLighting;
    float shininess;
    float3 pad1;
    float4x4 uvTransform;
    int isHalfVector;
    int useMonsterBallDistortion;
    float distortionStrength;
    float distortionFalloff;
    float4x4 environmentMatrix;
};

ConstantBuffer<Material> gMaterial : register(b0);
Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

void main(Object3dVertexShaderOutput input)
{
    float4 textureColor = gTexture.Sample(gSampler, input.texcoord);
    float alpha = textureColor.a * gMaterial.color.a;

    // シャドウマップ生成時もアルファを考慮し、板ポリの四角い影を防ぐ。
    clip(alpha - 0.1f);
}