
cbuffer cb
{
    float4 face_colors[4];
};
float3 ambient = { 0.1f, 0.1f, 0.1f };
float3 materialColor = { 0.01f, 0.0f, 0.0f };

float4 main() : SV_Target
{
    
    float3 lighting = ambient;
    float3 color = materialColor * ambient;
    return float4(color, 1.0f);
}