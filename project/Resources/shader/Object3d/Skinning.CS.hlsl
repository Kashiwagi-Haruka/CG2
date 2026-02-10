struct Vertex
{
    float4 position;
    float4 texcoord;
    float4 normal;
};
struct VertexInfluence
{
    float4 weight;
    int4 index;
};
struct SkinningInformation
{
    int numVertices;
};
StructuredBuffer<Well> gMatrixPalette : register(t0);
StructuredBuffer<Vertex> gInputVertices : register(t1);
StructuredBuffer<VertexInfluence> gInfluence : register(t2);
RWStructuredBuffer<Vertex> gOutputVertices : register(u0);
ConstantBuffer<SkinningInformation> gSkinningInformation : register(b0);
[numthreads(1024, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    int vertexIndex = DTid.x;
    if(vertexIndex < gSkinningInformation.numVertices)
    {
        
    }
    
    
}