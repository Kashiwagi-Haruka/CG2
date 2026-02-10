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
    if (particleIndex >= kMaxParticles)
    {
        return;
    }

    Particle particle = gParticles[particleIndex];
    if (particle.color.a == 0.0f)
    {
        return;
    }

    particle.velocity += gEmitter.acceleration * gPerFrame.deltaTime;
    particle.translate += particle.velocity * gPerFrame.deltaTime;
    particle.currentTime += gPerFrame.deltaTime;

    if (particle.lifeTime > 0.0f)
    {
        float alpha = 1.0f - (particle.currentTime / particle.lifeTime);
        particle.color.a = saturate(alpha);
    }
    else
    {
        particle.color.a = 0.0f;
    }

    gParticles[particleIndex] = particle;

}