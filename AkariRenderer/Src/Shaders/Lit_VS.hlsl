struct Matrices
{
    matrix ModelMatrix;
    matrix ViewMatrix;
    matrix ProjectionMatrix;
    matrix MVP;
};

ConstantBuffer<Matrices> MatCB : register( b0 );

struct VertexPositionNormalTangentBitangentTexture
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 Bitangent : BITANGENT;
    float3 TexCoord : TEXCOORD;
};

struct VertexShaderOutput
{
    float4 Position : SV_POSITION;
};

VertexShaderOutput main(VertexPositionNormalTangentBitangentTexture IN)
{
    VertexShaderOutput OUT;

    OUT.Position = mul(MatCB.MVP, float4(IN.Position, 1.0f));

    return OUT;
}
