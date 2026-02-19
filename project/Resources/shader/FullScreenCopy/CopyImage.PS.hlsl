#include "CopyImage.hlsli"

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    output.color = gTexture.Sample(gSampler, input.texcoord);

    float2 centeredUv = input.texcoord * (1.0f - input.texcoord.yx);
    float vignette = centeredUv.x * centeredUv.y * 16.0f;
    vignette = saturate(pow(vignette, 0.8f));
    float vignetteFactor = lerp(1.0f, vignette, saturate(vignetteStrength));
    output.color.rgb *= vignetteFactor;

    return output;
}