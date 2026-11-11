Texture2D colorMap_ : register(t0);
SamplerState colorSampler_ : register(s0);

struct VS_input
{
    float4 pos : POSITION;      //必须与输入布局里的语义名一致（不能写成 POSITIONT）
    float2 tex0 : TEXCOORD0;    //必须与输入布局的 R32G32_FLOAT 对应
};
struct PS_input
{
    float4 pos : SV_POSITION;
    float2 tex0 : TEXCOORD0;
};

PS_input VS_Main(VS_input vertex)
{
    PS_input vsOut = (PS_input) 0;
    vsOut.pos = vertex.pos;
    vsOut.tex0 = vertex.tex0;
    return vsOut;
}

float4 PS_Main(PS_input frag) : SV_TARGET
{
    return colorMap_.Sample(colorSampler_, frag.tex0);
}
