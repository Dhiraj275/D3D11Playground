
cbuffer cb
{
    float4 face_colors[4];
};

float4 main(uint tid: SV_PrimitiveID) : SV_Target
{
    return float4(face_colors[tid/2]);
}