struct VSOut
{
    float3 worldPos : Position;
    float3 normal : Normal;
    float4 pos : SV_Position;
};
cbuffer cbuf
{
    matrix transform;
};

VSOut main(float3 pos : Position, float3 n: Normal)
{
    VSOut vso;
    vso.worldPos = (float3) mul(float4(pos, 1.0f), transform);
    vso.normal = mul(n, (float3x3) transform);
    vso.pos = mul(float4(pos, 1.0f), transform);
    return vso;
}