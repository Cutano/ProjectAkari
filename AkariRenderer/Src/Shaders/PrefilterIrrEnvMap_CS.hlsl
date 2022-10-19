TextureCube<float4> Skybox : register(t0);
RWTexture2DArray<float4> Out : register(u0);

static const float PI = 3.141592653589793;

SamplerState LinearClampSampler : register(s0);

[numthreads(8, 8, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    float texelSize0 = 1.0 / 128.0;
    float2 uv = texelSize0 * (DTid.xy + 0.5) - 0.5;
    float3 sampleNormal[6];
    sampleNormal[0] = float3(0.5, -uv.y, -uv.x);
    sampleNormal[1] = float3(-0.5, -uv.y, uv.x);
    sampleNormal[2] = float3(uv.x, 0.5, uv.y);
    sampleNormal[3] = float3(uv.x, -0.5, -uv.y);
    sampleNormal[4] = float3(uv.x, -uv.y, 0.5);
    sampleNormal[5] = float3(-uv.x, -uv.y, -0.5);

    float3 N = normalize(sampleNormal[DTid.z]);

    float3 irradiance = 0.0f;   
    
    // tangent space calculation from origin point
    float3 up    = float3(0.0, 1.0, 0.0);
    float3 right = normalize(-cross(up, N));
    up         = normalize(-cross(N, right));
       
    float sampleDelta = 0.0125;
    float nrSamples = 0.0;
    for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
            // spherical to cartesian (in tangent space)
            float3 tangentSample = float3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
            // tangent space to world
            float3 sampleVec = normalize(tangentSample.x * right + tangentSample.y * up + tangentSample.z * N); 

            irradiance += Skybox.SampleLevel(LinearClampSampler, sampleVec, 0).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }
    irradiance = PI * irradiance * (1.0 / float(nrSamples));
    
    Out[DTid.xyz] = float4(irradiance, 1.0f);
}