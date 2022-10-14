TextureCube<float4> Skybox : register(t0);

// Write up to 4 mip map levels.
RWTexture2D<float4> OutMip1 : register(u0);
RWTexture2D<float4> OutMip2 : register(u1);
RWTexture2D<float4> OutMip3 : register(u2);
RWTexture2D<float4> OutMip4 : register(u3);

SamplerState LinearClampSampler : register(s0);

static const float PI = 3.141592653589793;

// ----------------------------------------------------------------------------
float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = saturate(dot(N, H));
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = NdotH2 * (a2 - 1.0) + 1.0;
    denom = PI * denom * denom;

    return nom / denom;
}

float RadicalInverse_VdC(uint bits) 
{
    bits = bits << 16u | bits >> 16u;
    bits = (bits & 0x55555555u) << 1u | (bits & 0xAAAAAAAAu) >> 1u;
    bits = (bits & 0x33333333u) << 2u | (bits & 0xCCCCCCCCu) >> 2u;
    bits = (bits & 0x0F0F0F0Fu) << 4u | (bits & 0xF0F0F0F0u) >> 4u;
    bits = (bits & 0x00FF00FFu) << 8u | (bits & 0xFF00FF00u) >> 8u;
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

float2 Hammersley(uint i, uint N)
{
    return float2(float(i) / float(N), RadicalInverse_VdC(i));
}

float3 ImportanceSampleGGX(float2 Xi, float3 N, float roughness)
{
    float a = roughness*roughness;
	
    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta*cosTheta);
	
    // from spherical coordinates to cartesian coordinates - halfway vector
    float3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;
	
    // from tangent-space H vector to world-space sample vector
    float3 up        = abs(N.z) < 0.999 ? float3(0.0, 0.0, 1.0) : float3(1.0, 0.0, 0.0);
    float3 tangent   = normalize(cross(up, N));
    float3 bitangent = cross(N, tangent);
	
    float3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(sampleVec);
}

float3 Calculate(uint mip, float3 N, float3 V)
{
    const uint SAMPLE_COUNT = 1024u;
    
    float roughness = (float)mip / 4.0;
    float3 prefilteredColor = 0.0;
    float totalWeight = 0.0;

    for (uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        // generates a sample vector that's biased towards the preferred alignment direction (importance sampling).
        float2 Xi = Hammersley(i, SAMPLE_COUNT);
        float3 H = ImportanceSampleGGX(Xi, N, roughness);
        float3 L  = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = max(dot(N, L), 0.0);
        if (NdotL > 0.0)
        {
            // sample from the environment's mip level based on roughness/pdf
            float D   = DistributionGGX(N, H, roughness);
            float NdotH = max(dot(N, H), 0.0);
            float HdotV = max(dot(H, V), 0.0);
            float pdf = D * NdotH / (4.0 * HdotV) + 0.0001; 

            float resolution = 1024.0; // resolution of source cubemap (per face)
            float saTexel  = 4.0 * PI / (6.0 * resolution * resolution);
            float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);

            float mipLevel = roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel); 

            prefilteredColor += Skybox.SampleLevel(LinearClampSampler, L, mipLevel).rgb * NdotL;
            totalWeight      += NdotL;
        }
    }

    prefilteredColor = prefilteredColor / totalWeight;
    return prefilteredColor;
}

// Dispatch(32, 32, 6)
// 
// TexelSize = 1 / 1024 Mip0
//           = 1 / 512  Mip1
//           = 1 / 256  Mip2
//           = 1 / 128  Mip3
//           = 1 / 64   Mip4
// UV = TexelSize * (SV_DispatchThreadID.xy + 0.5)
// CubeMapIndex = SV_DispatchThreadID.z
// Sampling: float3(uv, CubeMapIndex)
[numthreads(32, 32, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    float texelSize0 = 1.0 / 1024.0;
    float2 uv = texelSize0 * (DTid.xy + 0.5) - 0.5;
    float3 sampleNormal[6];
    sampleNormal[0] = float3(0.5, uv.y, -uv.x);
    sampleNormal[1] = float3(-0.5, uv.y, uv.x);
    sampleNormal[2] = float3(uv.x, 0.5, uv.y);
    sampleNormal[3] = float3(uv.x, -0.5, -uv.y);
    sampleNormal[4] = float3(uv.x, -uv.y, 0.5);
    sampleNormal[5] = float3(-uv.x, -uv.y, -0.5);

    float3 N = normalize(sampleNormal[DTid.z]);
    // make the simplyfying assumption that V equals R equals the normal 
    float3 R = N;
    float3 V = R;

    OutMip1[DTid.xy / 2] = float4(Calculate(1, N, V), 1.0);
    OutMip2[DTid.xy / 4] = float4(Calculate(2, N, V), 1.0);
    OutMip3[DTid.xy / 8] = float4(Calculate(3, N, V), 1.0);
    OutMip4[DTid.xy / 16] = float4(Calculate(4, N, V), 1.0);
}
