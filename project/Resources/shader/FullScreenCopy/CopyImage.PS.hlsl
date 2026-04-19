#include "CopyImage.hlsli"

Texture2D<float4> gTexture : register(t0);
Texture2D<float4> gOutlineTexture : register(t1);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

float rand2dTo1d(float2 value)
{
    const float2 smallValue = sin(value);
    const float2 dotDir = float2(12.9898f, 78.233f);
    float random = dot(smallValue, dotDir);
    return frac(sin(random) * 143758.5453f);
}

float4 ComputeOutline(float2 uv)
{
    float4 center = gOutlineTexture.Sample(gSampler, uv);
    if (center.a > 0.0f)
    {
        return float4(0.0f, 0.0f, 0.0f, 0.0f);
    }

    uint textureWidth;
    uint textureHeight;
    gOutlineTexture.GetDimensions(textureWidth, textureHeight);
    float2 texelSize = 1.0f / float2(textureWidth, textureHeight);

    const int kMaxOutlineWidthPixels = 20;
    float bestBlend = 0.0f;
    float3 bestColor = float3(0.0f, 0.0f, 0.0f);

    [loop]
    for (int y = -kMaxOutlineWidthPixels; y <= kMaxOutlineWidthPixels; ++y)
    {
        [loop]
        for (int x = -kMaxOutlineWidthPixels; x <= kMaxOutlineWidthPixels; ++x)
        {
            if (x == 0 && y == 0)
            {
                continue;
            }

            float distanceToCenter = length(float2((float) x, (float) y));
            if (distanceToCenter > kMaxOutlineWidthPixels)
            {
                continue;
            }

            float2 sampleUv = saturate(uv + float2((float) x, (float) y) * texelSize);
            float4 sampleColor = gOutlineTexture.Sample(gSampler, sampleUv);
            if (sampleColor.a <= 0.0f)
            {
                continue;
            }

            float sampleWidthPixels = sampleColor.a * kMaxOutlineWidthPixels;
            if (distanceToCenter > sampleWidthPixels)
            {
                continue;
            }

            float blend = saturate((sampleWidthPixels - distanceToCenter + 1.0f) / max(sampleWidthPixels, 1.0f));
            if (blend > bestBlend)
            {
                bestBlend = blend;
                bestColor = sampleColor.rgb;
            }
        }
    }

    return float4(bestColor, bestBlend);
}

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    output.color = gTexture.Sample(gSampler, input.texcoord);

    float2 centeredUv = input.texcoord * (1.0f - input.texcoord.yx);
    float vignette = centeredUv.x * centeredUv.y * 16.0f;
    vignette = saturate(pow(vignette, 0.8f));
    float vignetteFactor = lerp(1.0f, vignette, saturate(vignetteStrength));
    output.color.rgb *= vignetteFactor;

    if (randomNoiseEnabled > 0.5f)
    {
        float random = rand2dTo1d(input.texcoord * randomNoiseScale * randomNoiseTime);
        float3 noiseColor = float3(random, random, random);

        if (randomNoiseBlendMode < 0.5f)
        {
            output.color.rgb = noiseColor;
        }
        else if (randomNoiseBlendMode < 1.5f)
        {
            output.color.rgb += noiseColor;
        }
        else if (randomNoiseBlendMode < 2.5f)
        {
            output.color.rgb -= noiseColor;
        }
        else if (randomNoiseBlendMode < 3.5f)
        {
            output.color.rgb *= noiseColor;
        }
        else
        {
            output.color.rgb = 1.0f - ((1.0f - output.color.rgb) * (1.0f - noiseColor));
        }

        output.color.rgb = saturate(output.color.rgb);
    }

    float4 outline = ComputeOutline(input.texcoord);
    output.color.rgb = lerp(output.color.rgb, outline.rgb, outline.a);

    return output;
}