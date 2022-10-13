#include "pch.h"
#include "RenderStateObject.h"

#include "RHI/CommandList.h"
#include "RHI/Device.h"
#include "RHI/RenderTarget.h"
#include "RHI/RootSignature.h"
#include "SceneComponents/Material.h"

namespace Akari
{
    RenderStateObject::RenderStateObject(std::shared_ptr<Device> device) : m_Device(device)
    {
        // Create a root signature.
        // Allow input layout and deny unnecessary access to certain pipeline stages.
        const D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
                                                              D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
                                                              D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
                                                              D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

        // Descriptor range for the textures.
        const CD3DX12_DESCRIPTOR_RANGE1 descriptorRage(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, static_cast<UINT>(Material::TextureType::NumTypes), 3);

        CD3DX12_ROOT_PARAMETER1 rootParameters[NumRootParameters];
        rootParameters[MatricesCB].InitAsConstantBufferView( 0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL );
        rootParameters[MaterialCB].InitAsConstantBufferView( 0, 1, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL );
        rootParameters[LightPropertiesCB].InitAsConstants( sizeof( LightProperties ) / 4, 1, 0, D3D12_SHADER_VISIBILITY_PIXEL );
        rootParameters[PointLights].InitAsShaderResourceView( 0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL );
        rootParameters[SpotLights].InitAsShaderResourceView( 1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL );
        rootParameters[DirectionalLights].InitAsShaderResourceView( 2, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL );
        rootParameters[Textures].InitAsDescriptorTable( 1, &descriptorRage, D3D12_SHADER_VISIBILITY_PIXEL );

        const CD3DX12_STATIC_SAMPLER_DESC anisotropicSampler( 0, D3D12_FILTER_ANISOTROPIC );

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
        rootSignatureDescription.Init_1_1(NumRootParameters, rootParameters, 1, &anisotropicSampler, rootSignatureFlags );

        m_RootSig = device->CreateRootSignature( rootSignatureDescription.Desc_1_1 );

        // Create an SRV that can be used to pad unused texture slots.
        D3D12_SHADER_RESOURCE_VIEW_DESC defaultSRV;
        defaultSRV.Format                        = DXGI_FORMAT_R8G8B8A8_UNORM;
        defaultSRV.ViewDimension                 = D3D12_SRV_DIMENSION_TEXTURE2D;
        defaultSRV.Texture2D.MostDetailedMip     = 0;
        defaultSRV.Texture2D.MipLevels           = 1;
        defaultSRV.Texture2D.PlaneSlice          = 0;
        defaultSRV.Texture2D.ResourceMinLODClamp = 0;
        defaultSRV.Shader4ComponentMapping       = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

        m_DefaultSRV = m_Device->CreateShaderResourceView( nullptr, &defaultSRV );
    }

    RenderStateObject::~RenderStateObject()
    {
    }

    void RenderStateObject::SetModelMatrix(glm::mat4 modelMat)
    {
        m_MVP.Model = modelMat;
    }

    void RenderStateObject::SetViewMatrix(glm::mat4 viewMat)
    {
        m_MVP.View = viewMat;
    }

    void RenderStateObject::SetProjMatrix(glm::mat4 projMat)
    {
        m_MVP.Proj = projMat;
    }

    void RenderStateObject::SetDirectionalLights(const std::vector<DirectionalLight>& dirLights)
    {
        m_DirLights = dirLights;
    }

    void RenderStateObject::SetMaterial(const std::shared_ptr<Material>& mat)
    {
        m_Material = mat;
    }

    void RenderStateObject::SetRenderTarget(const std::shared_ptr<RenderTarget>& rt)
    {
        m_RenderTarget = rt;
    }

    void RenderStateObject::SetShader(const unsigned char* VSByteCode, size_t VSLength, const unsigned char* PSByteCode, size_t PSLength)
    {
        // Setup the pipeline state.
        struct PipelineStateStream
        {
            CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE        pRootSignature;
            CD3DX12_PIPELINE_STATE_STREAM_VS                    VS;
            CD3DX12_PIPELINE_STATE_STREAM_PS                    PS;
            CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER            RasterizerState;
            CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT          InputLayout;
            CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY    PrimitiveTopologyType;
            CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT  DSVFormat;
            CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
            CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC           SampleDesc;
            CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL         DepthStencilState;
        } pipelineStateStream;

        D3D12_RASTERIZER_DESC rasterizerDesc;
        rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
        rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
        rasterizerDesc.FrontCounterClockwise = FALSE;
        rasterizerDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
        rasterizerDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
        rasterizerDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
        rasterizerDesc.DepthClipEnable = true;
        rasterizerDesc.MultisampleEnable = true;
        rasterizerDesc.AntialiasedLineEnable = FALSE;
        rasterizerDesc.ForcedSampleCount = 0;
        rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

        D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {};
        depthStencilDesc.DepthEnable = true;
        depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_GREATER;
        depthStencilDesc.StencilEnable = false;

        pipelineStateStream.pRootSignature        = m_RootSig->GetD3D12RootSignature().Get();
        pipelineStateStream.VS                    = {VSByteCode, VSLength};
        pipelineStateStream.PS                    = {PSByteCode, PSLength};
        pipelineStateStream.RasterizerState       = CD3DX12_RASTERIZER_DESC(rasterizerDesc);
        pipelineStateStream.InputLayout           = VertexPositionNormalTangentBitangentTexture::InputLayout;
        pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        pipelineStateStream.DSVFormat             = m_RenderTarget->GetDepthStencilFormat();
        pipelineStateStream.RTVFormats            = m_RenderTarget->GetRenderTargetFormats();
        pipelineStateStream.SampleDesc            = m_RenderTarget->GetSampleDesc();
        pipelineStateStream.DepthStencilState     = CD3DX12_DEPTH_STENCIL_DESC(depthStencilDesc);

        m_PipelineStateObject = m_Device->CreatePipelineStateObject( pipelineStateStream );
    }

    void RenderStateObject::Apply(CommandList& cmd)
    {
        m_MVP.Mvp = m_MVP.Proj * m_MVP.View * m_MVP.Model;
        m_MVP.InvModel = inverse(m_MVP.Model);
        m_MVP.InvView = inverse(m_MVP.View);
        
        const auto& materialProps = m_Material->GetMaterialProperties();
        LightProperties lightProps;
        lightProps.NumDirectionalLights = static_cast<uint32_t>(m_DirLights.size());
        
        cmd.SetPipelineState(m_PipelineStateObject);
        cmd.SetGraphicsRootSignature(m_RootSig);
        cmd.SetGraphicsDynamicConstantBuffer(MatricesCB, m_MVP);
        cmd.SetGraphicsDynamicConstantBuffer(MaterialCB, materialProps);
        cmd.SetGraphics32BitConstants(LightPropertiesCB, lightProps);
        cmd.SetGraphicsDynamicStructuredBuffer(DirectionalLights, m_DirLights);

        using TextureType = Material::TextureType;

        BindTexture(cmd, static_cast<uint32_t>(TextureType::BaseColor), m_Material->GetTexture(TextureType::BaseColor));
        BindTexture(cmd, static_cast<uint32_t>(TextureType::Metallic), m_Material->GetTexture(TextureType::Metallic));
        BindTexture(cmd, static_cast<uint32_t>(TextureType::Roughness), m_Material->GetTexture(TextureType::Roughness));
        BindTexture(cmd, static_cast<uint32_t>(TextureType::Emissive), m_Material->GetTexture(TextureType::Emissive));
        BindTexture(cmd, static_cast<uint32_t>(TextureType::Occlusion), m_Material->GetTexture(TextureType::Occlusion));
        BindTexture(cmd, static_cast<uint32_t>(TextureType::Normal), m_Material->GetTexture(TextureType::Normal));
        BindTexture(cmd, static_cast<uint32_t>(TextureType::Bump), m_Material->GetTexture(TextureType::Bump));
        BindTexture(cmd, static_cast<uint32_t>(TextureType::Opacity), m_Material->GetTexture(TextureType::Opacity));
    }

    void RenderStateObject::BindTexture(CommandList& cmd, uint32_t offset, const std::shared_ptr<Texture>& tex) const
    {
        if ( tex )
        {
            cmd.SetShaderResourceView(Textures, offset, tex,
                                      D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE );
        }
        else
        {
            cmd.SetShaderResourceView(Textures, offset, m_DefaultSRV,
                                      D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE );
        }
    }
}
