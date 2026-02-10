#include "Particle.hlsli"

static const uint kMaxParticles = 4096;

RWStructuredBuffer<Particle> gParticles : register(u0);
RWStructuredBuffer<int> gFreeListIndex : register(u1);
RWStructuredBuffer<uint> gFreeList : register(u2);
ConstantBuffer<EmitterSphere> gEmitter : register(b0);
ConstantBuffer<PerFrame> gPerFrame : register(b1);

[numthreads(256, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint particleIndex = DTid.x;

    if (gEmitter.emit == 2)
    {
        if (particleIndex < kMaxParticles)
        {
            gParticles[particleIndex] = (Particle) 0;
            gFreeList[particleIndex] = particleIndex;
        }

        if (particleIndex == 0)
        {
            gFreeListIndex[0] = int(kMaxParticles) - 1;
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

    RandomGenerator generator;
    generator.seed = (float(particleIndex) + gPerFrame.time).xxx * max(gPerFrame.time, 0.0001f);

    for (uint countIndex = 0; countIndex < gEmitter.count; ++countIndex)
    {
        int freeListIndex;
        InterlockedAdd(gFreeListIndex[0], -1, freeListIndex);
        if (0 <= freeListIndex && freeListIndex < int(kMaxParticles))
        {
            int emitIndex = gFreeList[freeListIndex];

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
        else
        {
            InterlockedAdd(gFreeListIndex[0], 1);
            break;
        }
    }
}