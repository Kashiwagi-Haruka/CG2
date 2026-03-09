struct InstanceData
{
    float3 position;
    float yaw;
    float scale;
    float3 padding;
};

RWStructuredBuffer<InstanceData> gInstanceData : register(u0);
RWStructuredBuffer<float4x4> gInstanceWorld : register(u1);

cbuffer CollisionParams : register(b0)
{
    uint gInstanceCount;
    float gMinDistance;
    float gRoomMinX;
    float gRoomMaxX;
    float gRoomMinZ;
    float gRoomMaxZ;
    float gFloorY;
    float gPad0;
};

float4x4 MakeWorld(float3 position, float yaw, float scale)
{
    const float s = sin(yaw);
    const float c = cos(yaw);

    return float4x4(
        scale * c, 0.0f, -scale * s, 0.0f,
        0.0f, scale, 0.0f, 0.0f,
        scale * s, 0.0f, scale * c, 0.0f,
        position.x, position.y, position.z, 1.0f);
}

[numthreads(64, 1, 1)]
void main(uint3 dtid : SV_DispatchThreadID)
{
    const uint index = dtid.x;
    if (index >= gInstanceCount)
    {
        return;
    }

    InstanceData instance = gInstanceData[index];
    float2 pos = instance.position.xz;
    float2 push = float2(0.0f, 0.0f);

    [loop]
    for (uint i = 0; i < gInstanceCount; ++i)
    {
        if (i == index)
        {
            continue;
        }

        const float2 other = gInstanceData[i].position.xz;
        float2 delta = pos - other;
        const float distSq = dot(delta, delta);
        const float minDistSq = gMinDistance * gMinDistance;

        if (distSq < minDistSq && distSq > 1e-7f)
        {
            const float dist = sqrt(distSq);
            const float overlap = gMinDistance - dist;
            push += (delta / dist) * overlap * 0.5f;
        }
    }

    pos += push;
    pos.x = clamp(pos.x, gRoomMinX, gRoomMaxX);
    pos.y = clamp(pos.y, gRoomMinZ, gRoomMaxZ);

    instance.position.x = pos.x;
    instance.position.y = gFloorY;
    instance.position.z = pos.y;

    gInstanceData[index] = instance;
    gInstanceWorld[index] = MakeWorld(instance.position, instance.yaw, instance.scale);
}