#include "Skybox.hlsli"
#include "../Object3d/Object3d.hlsli"

TextureCube<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
	float4 color : SV_Target0;
};

struct Material
{
	float4 color;
	int enableLighting;
	float shininess;
	float environmentCoefficient;
	int useGrayscale;
	int useSepia;
	float distortionStrength;
	float distortionFalloff;
	float2 uvOffset;
	float2 uvScale;
};
ConstantBuffer<Material> gMaterial : register(b0);
PixelShaderOutput main(SkyboxVertexShaderOutput input){
	PixelShaderOutput output;
	float4 textureColor = gTexture.Sample(gSampler, input.texcoord);
	output.color = textureColor * gMaterial.color;
	return output;
}