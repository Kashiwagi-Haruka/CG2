struct Object3dVertexShaderOutput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
    float3 worldPosition : POSITION0;
    float4 directionalShadowPosition : TEXCOORD1;
    float4 pointShadowPosition : TEXCOORD2;
    float4 spotShadowPosition : TEXCOORD3;
    float4 areaShadowPosition : TEXCOORD4;
};