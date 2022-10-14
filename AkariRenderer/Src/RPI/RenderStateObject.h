#pragma once

#include "SceneComponents/Light.h"

namespace Akari
{
    class Device;
    class Texture;
    class Material;
    class CommandList;
    class RenderTarget;
    class RootSignature;
    class PipelineStateObject;
    class ShaderResourceView;

    class RenderStateObject
    {
    public:
        // An enum for root signature parameters.
        // I'm not using scoped enums to avoid the explicit cast that would be required
        // to use these as root indices in the root signature.
        enum RootParameters
        {
            // Vertex shader parameter
            MatricesCB,  // ConstantBuffer<Matrices> MatCB : register(b0);

            // Pixel shader parameters
            MaterialCB,         // ConstantBuffer<Material> MaterialCB : register( b0, space1 );
            LightPropertiesCB,  // ConstantBuffer<LightProperties> LightPropertiesCB : register( b1 );

            PointLights,        // StructuredBuffer<PointLight> PointLights : register( t0 );
            SpotLights,         // StructuredBuffer<SpotLight> SpotLights : register( t1 );
            DirectionalLights,  // StructuredBuffer<DirectionalLight> DirectionalLights : register( t2 )

            Textures,  // Texture2D AmbientTexture       : register( t3 );
                       // Texture2D EmissiveTexture : register( t4 );
                       // Texture2D DiffuseTexture : register( t5 );
                       // Texture2D SpecularTexture : register( t6 );
                       // Texture2D SpecularPowerTexture : register( t7 );
                       // Texture2D NormalTexture : register( t8 );
                       // Texture2D BumpTexture : register( t9 );
                       // Texture2D OpacityTexture : register( t10 );
            CubeMaps,  // TextureCube<float4> Skybox : register( t11 );
                       // TextureCube<float4> SkyboxIrr : register( t12 );
            LUTs,      // Texture2D IBLTexture : register( t13 );
            NumRootParameters
        };

        // Light properties for the pixel shader.
        struct LightProperties
        {
            uint32_t NumPointLights{0};
            uint32_t NumSpotLights{0};
            uint32_t NumDirectionalLights{0};
        };
        
        RenderStateObject(std::shared_ptr<Device> device);
        ~RenderStateObject();

        void SetModelMatrix(glm::mat4 modelMat);
        void SetViewMatrix(glm::mat4 viewMat);
        void SetProjMatrix(glm::mat4 projMat);

        void SetDirectionalLights(const std::vector<DirectionalLight>& dirLights);
        void SetCubeMaps(const std::shared_ptr<ShaderResourceView>& skyboxSRV, std::shared_ptr<ShaderResourceView> skyboxIrrSRV);
        void SetLUTs(const std::shared_ptr<ShaderResourceView>& IBLTextureSRV);
        void SetMaterial(const std::shared_ptr<Material>& mat);
        void SetRenderTarget(const std::shared_ptr<RenderTarget>& rt);
        void SetShader(const unsigned char* VSByteCode, size_t VSLength, const unsigned char* PSByteCode, size_t PSLength);

        void Apply(CommandList& cmd);

    private:
        struct alignas(16) MVP
        {
            glm::mat4 Model = glm::mat4(1.0);
            glm::mat4 View = glm::mat4(1.0);
            glm::mat4 Proj = glm::mat4(1.0);
            glm::mat4 Mvp = glm::mat4(1.0);
            glm::mat4 InvModel = glm::mat4(1.0);
            glm::mat4 InvView = glm::mat4(1.0);

            glm::mat4 operator() () const {return Proj * View * Model;}
            operator glm::mat4() const {return Proj * View * Model;}
        };

        inline void BindTexture(CommandList& cmd, uint32_t offset, const std::shared_ptr<Texture>& tex) const;

        MVP m_MVP;
        
        std::shared_ptr<Device> m_Device;
        std::shared_ptr<Material> m_Material;
        std::shared_ptr<RenderTarget> m_RenderTarget;
        std::shared_ptr<RootSignature> m_RootSig;
        std::shared_ptr<ShaderResourceView> m_DefaultSRV;
        std::shared_ptr<PipelineStateObject> m_PipelineStateObject;

        std::vector<DirectionalLight> m_DirLights;

        std::shared_ptr<ShaderResourceView> m_SkyboxSRV;
        std::shared_ptr<ShaderResourceView> m_SkyboxIrrSRV;
        std::shared_ptr<ShaderResourceView> m_IBLTextureSRV;
    };
    
}
