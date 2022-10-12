#include "pch.h"
#include "SkyboxPass.h"

#include "RHI/CommandList.h"
#include "RHI/Renderer.h"
#include "RHI/Texture.h"
#include "RHI/RootSignature.h"
#include "RHI/Device.h"
#include "RHI/RenderTarget.h"
#include "SceneComponents/Model.h"
#include "SceneComponents/Mesh.h"
#include "SceneComponents/Scene.h"
#include "SceneComponents/Camera/EditorCamera.h"
#include "Shaders/Generated/Skybox_VS.h"
#include "Shaders/Generated/Skybox_PS.h"

namespace Akari
{
    SkyboxPass::SkyboxPass(std::shared_ptr<RenderTarget> renderTarget) : RenderPass(renderTarget)
    {
               
        {
            const auto cmdCopy = Renderer::GetInstance().GetCommandListCopy();
            m_Skybox = cmdCopy->CreateCube(1.0f, true);
            m_SkyboxPano = cmdCopy->LoadTextureFromFile(L"Res/Textures/HDR/Subway_Lights_3k.hdr", true);
            Renderer::GetInstance().ExecuteCommandList(cmdCopy);
        }
        
        auto cubemapDesc  = m_SkyboxPano->GetD3D12ResourceDesc();
        cubemapDesc.Width = cubemapDesc.Height = 1024;
        cubemapDesc.DepthOrArraySize           = 6;
        cubemapDesc.MipLevels                  = 0;

        {
            const auto cmdCompute = Renderer::GetInstance().GetCommandListCompute();

            m_SkyboxCubemap = Renderer::GetInstance().GetDevice()->CreateTexture(cubemapDesc);
            m_SkyboxCubemap->SetName(L"Skybox Cubemap");

            cmdCompute->PanoToCubemap(m_SkyboxCubemap, m_SkyboxPano);
            Renderer::GetInstance().ExecuteCommandList(cmdCompute);
        }

        D3D12_SHADER_RESOURCE_VIEW_DESC cubeMapSRVDesc = {};
        cubeMapSRVDesc.Format                          = cubemapDesc.Format;
        cubeMapSRVDesc.Shader4ComponentMapping         = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        cubeMapSRVDesc.ViewDimension                   = D3D12_SRV_DIMENSION_TEXTURECUBE;
        cubeMapSRVDesc.TextureCube.MipLevels           = static_cast<UINT>(-1);  // Use all mips.

        m_SkyboxSRV = Renderer::GetInstance().GetDevice()->CreateShaderResourceView(m_SkyboxCubemap, &cubeMapSRVDesc);

        // Setup the input layout for the skybox vertex shader.
        D3D12_INPUT_ELEMENT_DESC inputLayout[1] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
              D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        };

        // Allow input layout and deny unnecessary access to certain pipeline stages.
        D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
                                                        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
                                                        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
                                                        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

        CD3DX12_DESCRIPTOR_RANGE1 descriptorRange( D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0 );

        CD3DX12_ROOT_PARAMETER1 rootParameters[NumRootParams];
        rootParameters[0].InitAsConstants( sizeof( DirectX::XMMATRIX ) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX );
        rootParameters[1].InitAsDescriptorTable( 1, &descriptorRange, D3D12_SHADER_VISIBILITY_PIXEL );

        CD3DX12_STATIC_SAMPLER_DESC linearClampSampler(
            0, D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP );

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
        rootSignatureDescription.Init_1_1( NumRootParams, rootParameters, 1, &linearClampSampler, rootSignatureFlags );

        m_RootSig = Renderer::GetInstance().GetDevice()->CreateRootSignature( rootSignatureDescription.Desc_1_1 );

        D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {};
        depthStencilDesc.DepthEnable = false;
        depthStencilDesc.StencilEnable = false;
        
        D3D12_RASTERIZER_DESC rasterizerDesc = {};
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

        // Setup the Skybox pipeline state.
        struct SkyboxPipelineState
        {
            CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE        pRootSignature;
            CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT          InputLayout;
            CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY    PrimitiveTopologyType;
            CD3DX12_PIPELINE_STATE_STREAM_VS                    VS;
            CD3DX12_PIPELINE_STATE_STREAM_PS                    PS;
            CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
            CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC           SampleDesc;
            CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER            RasterizerState;
            CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL         DepthStencilState;
        } skyboxPipelineStateStream {};

        skyboxPipelineStateStream.pRootSignature        = m_RootSig->GetD3D12RootSignature().Get();
        skyboxPipelineStateStream.InputLayout           = VertexPositionNormalTangentBitangentTexture::InputLayout;
        skyboxPipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        skyboxPipelineStateStream.VS                    = {g_Skybox_VS, sizeof g_Skybox_VS};
        skyboxPipelineStateStream.PS                    = {g_Skybox_PS, sizeof g_Skybox_PS};
        skyboxPipelineStateStream.RTVFormats            = m_RenderTarget->GetRenderTargetFormats();
        skyboxPipelineStateStream.SampleDesc            = m_RenderTarget->GetSampleDesc();
        skyboxPipelineStateStream.RasterizerState       = CD3DX12_RASTERIZER_DESC(rasterizerDesc);
        skyboxPipelineStateStream.DepthStencilState     = CD3DX12_DEPTH_STENCIL_DESC(depthStencilDesc);

        m_PipelineState = Renderer::GetInstance().GetDevice()->CreatePipelineStateObject( skyboxPipelineStateStream );
    }

    void SkyboxPass::Record(const RenderContext& context)
    {
        m_Cmd = Renderer::GetInstance().GetCommandListDirect();
        
        SkyboxVisitor visitor(*m_Cmd);
        const auto cam = context.scene->GetCamera();
        const auto view = lookAt({0, 0, 0}, cam->GetForwardDirection(), cam->GetUpDirection());
        const auto proj = cam->GetProjectionMatrix();

        m_Cmd->SetPipelineState(m_PipelineState);
        m_Cmd->SetGraphicsRootSignature(m_RootSig);
        m_Cmd->SetRenderTarget(*m_RenderTarget);
        m_Cmd->SetViewport(m_RenderTarget->GetViewport());
        m_Cmd->SetScissorRect(CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX));
        m_Cmd->SetGraphics32BitConstants(0, proj * view);
        m_Cmd->SetShaderResourceView(1, 0, m_SkyboxSRV, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

        m_Skybox->Accept(visitor);
    }

    void SkyboxPass::Execute()
    {
        Renderer::GetInstance().ExecuteCommandList(m_Cmd);
    }

    SkyboxVisitor::SkyboxVisitor(CommandList& commandList) : m_Cmd(commandList)
    {
    }

    void SkyboxVisitor::Visit(Scene& scene)
    {
    }

    void SkyboxVisitor::Visit(SceneObject& model)
    {
    }

    void SkyboxVisitor::Visit(ModelNode& modelNode)
    {
    }

    void SkyboxVisitor::Visit(Mesh& mesh)
    {
        mesh.Draw(m_Cmd);
    }
}
