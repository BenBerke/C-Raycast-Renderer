struct Input {
    float4 v_color : COLOR0;
};
struct Output {
    float4 FragColor : SV_Target0;
};

Output main(Input input) {
    Output output;
    output.FragColor = input.v_color;
    return output;
}