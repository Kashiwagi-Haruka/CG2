struct ShadowPayload
{
    bool hit;
};

RaytracingAccelerationStructure Scene : register(t0);

[shader("miss")]
void ShadowMiss(inout ShadowPayload payload)
{
    payload.hit = false;
}

[shader("closesthit")]
void ShadowClosestHit(inout ShadowPayload payload, BuiltInTriangleIntersectionAttributes attr)
{
    payload.hit = true;
}

float TraceHardShadow(float3 worldPos, float3 normal, float3 lightDir, float lightDistance)
{
    ShadowPayload payload;
    payload.hit = false;

    RayDesc ray;
    ray.Origin = worldPos + normal * 0.001f;
    ray.Direction = normalize(lightDir);
    ray.TMin = 0.001f;
    ray.TMax = lightDistance;

    TraceRay(
        Scene,
        RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH,
        0xFF,
        0,
        1,
        0,
        ray,
        payload
    );

    return payload.hit ? 0.0f : 1.0f;
}