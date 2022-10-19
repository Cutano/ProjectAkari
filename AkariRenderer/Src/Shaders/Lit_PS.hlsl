struct Matrices
{
	matrix ModelMatrix;
	matrix ViewMatrix;
	matrix ProjectionMatrix;
	matrix MVP;
	matrix InverseModelMatrix;
	matrix InverseViewMatrix;
};

struct LightProperties
{
	uint NumPointLights;
	uint NumSpotLights;
	uint NumDirectionalLights;
};

struct DirectionalLight
{
	float3 Translation;
	float3 Rotation;
	float3 Scale;
	float  Padding[3];
	float3 Radiance;
	float Intensity;
	float LightSize; // For PCSS
	float ShadowAmount;
	int CastShadows;
	int SoftShadows;
};

struct MaterialProperties
{
	float4 BaseColor;
	//------------------------------------ ( 16 bytes )
	float4 Emissive;
	//------------------------------------ ( 16 bytes )
	float Opacity;                       // If Opacity < 1, then the material is transparent.
	float Roughness;
	float Metallic;             
	float NormalScale;

	//------------------------------------ ( 16 bytes )
	uint HasBaseColorTexture;
	uint HasMetallicTexture;
	uint HasRoughnessTexture;
	uint HasEmissiveTexture;
	//------------------------------------ ( 16 bytes )
	uint HasOcclusionTexture;
	uint HasNormalTexture;
	uint HasBumpTexture;
	uint HasOpacityTexture;
	//------------------------------------ ( 16 bytes )
	// Total:                              ( 16 * 8 = 128 bytes )
};

struct VertexShaderOutput
{
	float4 PositionWS  : POSITION;
	float3 NormalWS    : NORMAL;
	float3 TangentWS   : TANGENT;
	float3 BitangentWS : BITANGENT;
	float2 TexCoord    : TEXCOORD;
	float4 Position    : SV_POSITION;
	bool IsFrontFace   : SV_IsFrontFace;
};

struct SurfaceShadingData
{
	float4 BaseColor;
	float4 Emissive;
	float3 Normal;
	float3 Tangent;
	float3 Bitangent;
	float Opacity;                       // If Opacity < 1, then the material is transparent.
	float Roughness;
	float Occlusion;
	float Metallic;             
};

ConstantBuffer<Matrices> MatCB : register(b0, space0);
ConstantBuffer<MaterialProperties> MaterialCB : register(b0, space1);
ConstantBuffer<LightProperties> LightPropertiesCB : register(b1);

StructuredBuffer<DirectionalLight> DirectionalLights : register(t2);

Texture2D BaseColor : register( t3 );
Texture2D Metallic : register( t4 );
Texture2D Roughness : register( t5 );
Texture2D Emissive : register( t6 );
Texture2D Occlusion : register( t7 );
Texture2D Normal : register( t8 );
Texture2D Bump : register( t9 );
Texture2D Opacity : register( t10 );
TextureCube<float4> Skybox : register( t11 );
TextureCube<float4> SkyboxIrr : register( t12 );
Texture2D IBLTexture : register( t13 );

SamplerState AnisotropicSampler : register(s0);
SamplerState LinearClampSampler : register(s1);

static const float PI = 3.141592653589793;

float3 fresnelSchlick(float cosTheta, float3 F0)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float3 fresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
	return F0 + (max(float3(1.0 - roughness.xxx), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}   

float AntiAliasingDistributionGGX(float3 N, float3 H, float roughness)
{
	float a      = roughness * roughness;
	float sigma = 0.50; //- screen space variance
	float Kappa = 0.18;	//- clamping threshold

	float2 footprint_bounding_box = fwidth(H.xy);
	float2 variance = sigma * sigma * footprint_bounding_box * footprint_bounding_box;
	float2 kernel_roughness = min(float2(Kappa, Kappa), 2.0 * variance);
	
	float a2     = a * a + kernel_roughness.x;
	float NdotH  = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;

	float nom   = a2;
	float denom = NdotH2 * (a2 - 1.0) + 1.0;
	denom = PI * denom * denom;

	return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = roughness + 1.0;
	float k = r * r / 8.0;

	float nom   = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return nom / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}

float3 DirectPBRLighting(float3 baseColor, float3 V, float3 L, float3 N, float3 F0, float roughness, float metallic)
{
	// Cook-Torrance BRDF
	F0 = lerp(F0, baseColor, metallic);
	float3 H = normalize(V + L);
	
	float NDF = AntiAliasingDistributionGGX(N, H, roughness);        
	float G   = GeometrySmith(N, V, L, roughness);      
	float3 F  = fresnelSchlick(max(dot(H, V), 0.0), F0);

	float3 nominator    = NDF * G * F;
	float  denominator  = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
	float3 specular = saturate(nominator / denominator);

	float3 kS = F;
	float3 kD = 1.0 - kS;
	kD *= 1.0 - metallic;

	float NoL = saturate(dot(N, L));

	return (kD * baseColor / PI + specular) * NoL;
}

float3 ImageBasedPBRLighting(float3 baseColor, float3 V, float3 N, float3 F0, float roughness, float metallic, float ao)
{
	// Cook-Torrance BRDF
	F0 = lerp(F0, baseColor, metallic);
	float3 F  = fresnelSchlickRoughness(saturate(dot(N, V)), F0, roughness);
	float3 R = reflect(-V, N); 

	float3 kS = F;
	float3 kD = 1.0 - kS;
	kD *= 1.0 - metallic;
	
	float3 irradiance = SkyboxIrr.Sample(LinearClampSampler, N).rgb;
	float3 diffuse    = irradiance * baseColor;

	const float MAX_REFLECTION_LOD = 10.0;
	float3 prefilteredColor = Skybox.SampleLevel(LinearClampSampler, R, roughness * MAX_REFLECTION_LOD).rgb;
	float2 brdf = IBLTexture.Sample(LinearClampSampler, float2(saturate(dot(N, V)), roughness)).rg;
	float3 specular = prefilteredColor * (F * brdf.x + brdf.y);
	
	float3 ambient = (kD * diffuse + specular) * ao;

	return ambient;
}

float3 ExpandNormal(float3 n)
{
	return n * 2.0f - 1.0f;
}

float3 NormalMapping(float3x3 TBN, float3 N)
{
	N = float3(N.r, 1.0f - N.g, N.b);
	N = ExpandNormal(N);
	N.xy *= MaterialCB.NormalScale;

	// Transform normal from tangent space to view space.
	N = mul(N, TBN);
	return normalize(N);
}

SurfaceShadingData GetSurfaceData(VertexShaderOutput psInput)
{
	SurfaceShadingData o;

	if (MaterialCB.HasBaseColorTexture)
	{
		o.BaseColor = BaseColor.Sample(AnisotropicSampler, psInput.TexCoord);
	}
	else
	{
		o.BaseColor = MaterialCB.BaseColor;
	}

	if (MaterialCB.HasRoughnessTexture && MaterialCB.HasMetallicTexture)
	{
		float3 temp1 = Roughness.Sample(AnisotropicSampler, psInput.TexCoord).rgb;
		float3 temp2 = Metallic.Sample(AnisotropicSampler, psInput.TexCoord).rgb;

		if (abs(length(temp1 - temp2)) < 0.001)
		{
			o.Metallic = temp2.b;
			o.Roughness = temp1.g;
		}
		else
		{
			o.Metallic = temp2.r;
			o.Roughness = temp1.r;
		}
	}
	else
	{
		if (MaterialCB.HasRoughnessTexture)
		{
			o.Roughness = Roughness.Sample(AnisotropicSampler, psInput.TexCoord).r;
		}
		else
		{
			o.Roughness = MaterialCB.Roughness;
		}

		if (MaterialCB.HasMetallicTexture)
		{
			o.Metallic = Metallic.Sample(AnisotropicSampler, psInput.TexCoord).r;
		}
		else
		{
			o.Metallic = MaterialCB.Metallic;
		}
	}

	if (MaterialCB.HasEmissiveTexture)
	{
		o.Emissive = Emissive.Sample(AnisotropicSampler, psInput.TexCoord);
	}
	else
	{
		o.Emissive = MaterialCB.Emissive;
	}

	if (MaterialCB.HasOpacityTexture)
	{
		o.Opacity = Opacity.Sample(AnisotropicSampler, psInput.TexCoord).r;
	}
	else
	{
		o.Opacity = MaterialCB.Opacity;
	}
	
	if (MaterialCB.HasOcclusionTexture)
	{
		o.Occlusion = Occlusion.Sample(AnisotropicSampler, psInput.TexCoord).r;
	}
	else
	{
		o.Occlusion = 1.0f;
	}

	if (MaterialCB.HasNormalTexture)
	{
		float3 tangent = normalize(psInput.TangentWS);
		float3 bitangent = normalize(psInput.BitangentWS);
		float3 normal = normalize(psInput.NormalWS);
		if (psInput.IsFrontFace == false)
		{
			tangent *= -1.0;
			bitangent *= -1.0;
			normal *= -1.0;
		}
		float3x3 TBN = float3x3(tangent, bitangent, normal);

		o.Normal = NormalMapping(TBN, Normal.SampleLevel(AnisotropicSampler, psInput.TexCoord, 0).rgb);
		o.Tangent = psInput.TangentWS;
		o.Bitangent = psInput.BitangentWS;
	}
	else
	{
		o.Normal = psInput.NormalWS;
		o.Tangent = psInput.TangentWS;
		o.Bitangent = psInput.BitangentWS;
	}
	
	return o;
}

float4 main(VertexShaderOutput psInput) : SV_TARGET
{
	SurfaceShadingData surface = GetSurfaceData(psInput);
	
	float3 normalWS = surface.Normal;
	float3 viewPosWS = MatCB.InverseViewMatrix._14_24_34;
	float3 viewDir = normalize(viewPosWS - psInput.PositionWS.xyz);
	float3 col = 0;
	for (uint i = 0; i < LightPropertiesCB.NumDirectionalLights; ++i)
	{
		float pitch = DirectionalLights[i].Rotation.x;
		float roll  = DirectionalLights[i].Rotation.y;
		float yaw   = DirectionalLights[i].Rotation.z;
		// float x = -cos(roll) * sin(yaw) - sin(roll) * sin(pitch) * cos(yaw);
		// float y = cos(pitch) * cos(yaw);
		// float z = -sin(roll) * sin(yaw) + cos(roll) * sin(pitch) * cos(yaw);
		float x = -sin(yaw);
		float y = cos(pitch) * cos(yaw);
		float z = sin(pitch) * cos(yaw);

		float3 lightDir = normalize(float3(x, y, z));
		col += DirectPBRLighting(surface.BaseColor.rgb,
			viewDir, lightDir, normalWS, 0.04f,
			surface.Roughness, surface.Metallic) * DirectionalLights[i].Radiance * DirectionalLights[i].Intensity;
	}
	col += ImageBasedPBRLighting(surface.BaseColor.rgb,
				viewDir, normalWS, 0.04f,
				surface.Roughness, surface.Metallic, 1.0);

	col += surface.Emissive.rgb;
	
	return float4(col, 1.0f);
}