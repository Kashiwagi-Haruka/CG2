struct InstanceData
{
    float3 position;
    float yaw;
    float scale;
    float velocityY;
    float radius;
    float2 padding;
};

RWStructuredBuffer<InstanceData> gInstanceData : register(u0);

cbuffer CollisionParams : register(b0)
{
    uint gInstanceCount;
    float gDeltaTime;
    float gRoomMinX;
    float gRoomMaxX;
    float gRoomMinZ;
    float gRoomMaxZ;
    float gFloorY;
    float gGravity;
    float gBounceDamping;
    float gSeparationBias;
    float2 gPad0;
};

[numthreads(64, 1, 1)]
void main(uint3 dtid : SV_DispatchThreadID)
{
    const uint index = dtid.x;
    if (index >= gInstanceCount)
    {
        return;
    }

    InstanceData instance = gInstanceData[index];

    instance.velocityY += gGravity * gDeltaTime;
    instance.position.y += instance.velocityY * gDeltaTime;
    if (instance.position.y <= gFloorY)
    {
        instance.position.y = gFloorY;
        instance.velocityY = -instance.velocityY * gBounceDamping;
    }

    float2 pos = instance.position.xz;
    float2 push = float2(0.0f, 0.0f);

    [loop]
    for (uint i = 0; i < gInstanceCount; ++i)
    {
        if (i == index)
        {
            continue;
        }

        const InstanceData otherData = gInstanceData[i];
        const float2 other = otherData.position.xz;
        const float minDist = instance.radius + otherData.radius;
        float2 delta = pos - other;
        const float distSq = dot(delta, delta);
        const float minDistSq = minDist * minDist;

        if (distSq < minDistSq && distSq > 1e-7f)
        {
            const float dist = sqrt(distSq);
            const float overlap = minDist - dist;
            push += (delta / dist) * (overlap * 0.5f + gSeparationBias);
        }
    }

    pos += push;
    pos.x = clamp(pos.x, gRoomMinX, gRoomMaxX);
    pos.y = clamp(pos.y, gRoomMinZ, gRoomMaxZ);

    instance.position.x = pos.x;
    instance.position.z = pos.y;

    gInstanceData[index] = instance;
}