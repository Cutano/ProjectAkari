cbuffer TexIdBuffer : register(b1)
{
    uint TexId;
}

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float4 col : COLOR0;
    float2 uv  : TEXCOORD0;
};

SamplerState sampler0 : register(s0);
Texture2D texture0 : register(t0);
Texture2D texture1 : register(t1);

float4 main(PS_INPUT input) : SV_Target
{
    float4 out_col = float4(0, 0, 0, 1);
    
    if (TexId == 0)
    {
        out_col = input.col * texture0.SampleLevel(sampler0, input.uv, 0);
    }
    if (TexId == 1)
    {
        out_col = input.col * texture1.SampleLevel(sampler0, input.uv, 0);
    }
    return out_col;
}
