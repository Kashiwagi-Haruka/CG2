#include "Particle.hlsli"

static const uint kMaxParticles = 1024;

RWStructuredBuffer<Particle> gParticles : register(u0);
RWStructuredBuffer<int> gFreeCounter : register(u1);
ConstantBuffer<EmitterSphere> gEmitter : register(b0);
ConstantBuffer<PerFrame> gPerFrame : register(b1);

[numthreads(1024, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint particleIndex = DTid.x;

    if (gEmitter.emit == 2)
    {
        if (particleIndex < kMaxParticles)
        {
            gParticles[particleIndex] = (Particle) 0;
        }

        if (particleIndex == 0)
        {
            gFreeCounter[0] = 0;
        }
        return;
    }

    if (particleIndex != 0)
    {
        return;
    }

    if (gEmitter.emit == 0)
    {
        return;
    }

    gFreeCounter[0] = 0;

    RandomGenerator generator;
    generator.seed = (float(particleIndex) + gPerFrame.time).xxx * max(gPerFrame.time, 0.0001f);

    for (uint countIndex = 0; countIndex < gEmitter.count; ++countIndex)
    {
        int emitIndex;
        InterlockedAdd(gFreeCounter[0], 1, emitIndex);
        if (emitIndex >= int(kMaxParticles))
        {
            break;
        }

        float3 randomDirection = generator.Generate3d() * 2.0f - 1.0f;
        float randomLength = generator.Generate1d() * gEmitter.radius;

        gParticles[emitIndex].scale = generator.Generate3d() * 0.4f + 0.1f;
        gParticles[emitIndex].translate = gEmitter.translate + randomDirection * randomLength;
        gParticles[emitIndex].velocity = (generator.Generate3d() * 2.0f - 1.0f) * 0.05f;
        gParticles[emitIndex].lifeTime = gEmitter.lifeTime;
        gParticles[emitIndex].currentTime = 0.0f;
        gParticles[emitIndex].color.rgb = generator.Generate3d();
        gParticles[emitIndex].color.a = 1.0f;
    }
}