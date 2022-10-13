struct Matrices
{
    matrix ModelMatrix;
    matrix ViewMatrix;
    matrix ProjectionMatrix;
    matrix MVP;
    matrix InverseViewMatrix;
};

ConstantBuffer<Matrices> MatCB : register(b0, space0);

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
    float4 PositionWS  : POSITION;
    float3 NormalWS    : NORMAL;
    float3 TangentWS   : TANGENT;
    float3 BitangentWS : BITANGENT;
    float2 TexCoord    : TEXCOORD;
    float4 Position    : SV_POSITION;
};

VertexShaderOutput main(VertexPositionNormalTangentBitangentTexture IN)
{
    VertexShaderOutput OUT;
    
    OUT.Position    = mul(MatCB.MVP, float4(IN.Position, 1.0f));
    OUT.PositionWS  = mul(MatCB.ModelMatrix, float4(IN.Position, 1.0f));
    OUT.NormalWS    = mul((float3x3)MatCB.ModelMatrix, IN.Normal);
    OUT.TangentWS   = mul((float3x3)MatCB.ModelMatrix, IN.Tangent);
    OUT.BitangentWS = mul((float3x3)MatCB.ModelMatrix, IN.Bitangent);
    OUT.TexCoord    = IN.TexCoord.xy;

    return OUT;
}
