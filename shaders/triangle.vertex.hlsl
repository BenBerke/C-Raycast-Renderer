struct Input
{
    float3 a_position : TEXCOORD0;
    float4 a_color : TEXCOORD1;
};

struct Output
{
    float4 v_position : SV_Position;
    float4 v_color : TEXCOORD0;
};

Output main(Input input)
{
    Output output;
    output.v_position = float4(input.a_position, 1.0f);
    output.v_color = input.a_color;
    return output;
}