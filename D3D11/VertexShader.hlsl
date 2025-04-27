struct VSOut
{
    float4 pos : SV_Position;
};
cbuffer cbuf
{
    matrix transform;
};

VSOut main(float3 pos : Position)
{
    VSOut vso;
    vso.pos = mul(float4(pos.x, pos.y, pos.z, 1.0f), transform);
    return vso;
}