#include "pch.h"
#include "ForwardPipeline.h"
#include "Pass/ForwardOpaquePass.h"
#include "Pass/GroundGridPass.h"

#include "RHI/Renderer.h"
#include "RHI/SwapChain.h"
#include "RHI/Texture.h"
#include "RHI/CommandList.h"
#include "RHI/Device.h"
#include "RPI/RenderContext.h"

namespace Akari
{
    ForwardPipeline::ForwardPipeline()
    {
        
    }

    void ForwardPipeline::Prepare()
    {
        {
            const auto cmdCopy = Renderer::GetInstance().GetCommandListCopy();

            spdlog::info("\tLoading cube maps...");
            m_SkyboxPano = cmdCopy->LoadTextureFromFile(L"Res/Textures/HDR/Subway_Lights_3k.hdr", true);
            // m_SkyboxIrrPano = cmdCopy->LoadTextureFromFile(L"Res/Textures/HDR/Subway_Lights_Env.hdr", true);
            m_IBLTexture = cmdCopy->LoadTextureFromFile(L"Res/Textures/LUT/IBL.png", false);
            Renderer::GetInstance().ExecuteCommandList(cmdCopy);
        }
        
        auto cubemapDesc  = m_SkyboxPano->GetD3D12ResourceDesc();
        cubemapDesc.Width = cubemapDesc.Height = 1024;
        cubemapDesc.DepthOrArraySize           = 6;
        cubemapDesc.MipLevels                  = 0;

        auto IBLDesc  = m_IBLTexture->GetD3D12ResourceDesc();

        {
            const auto cmdCompute = Renderer::GetInstance().GetCommandListCompute();
            spdlog::info("\tPreprocessing textures...");

            m_SkyboxCubemap = Renderer::GetInstance().GetDevice()->CreateTexture(cubemapDesc);
            m_SkyboxCubemap->SetName(L"Skybox Cubemap");

            cmdCompute->PanoToCubemap(m_SkyboxCubemap, m_SkyboxPano);

            auto cubemapIrrDesc  = m_SkyboxCubemap->GetD3D12ResourceDesc();
            cubemapIrrDesc.Width = cubemapIrrDesc.Height = 128;
            cubemapIrrDesc.DepthOrArraySize              = 6;
            cubemapIrrDesc.MipLevels                     = 1;

            m_SkyboxIrrCubemap = Renderer::GetInstance().GetDevice()->CreateTexture(cubemapIrrDesc);
            m_SkyboxIrrCubemap->SetName(L"Skybox Irradiance Cubemap");

            cmdCompute->PrefilterIrrCubeMap(m_SkyboxCubemap, m_SkyboxIrrCubemap);
            cmdCompute->PrefilterCubeMap(m_SkyboxCubemap);
            
            Renderer::GetInstance().ExecuteCommandList(cmdCompute);
        }

        D3D12_SHADER_RESOURCE_VIEW_DESC cubeMapSRVDesc = {};
        cubeMapSRVDesc.Format                          = cubemapDesc.Format;
        cubeMapSRVDesc.Shader4ComponentMapping         = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        cubeMapSRVDesc.ViewDimension                   = D3D12_SRV_DIMENSION_TEXTURECUBE;
        cubeMapSRVDesc.TextureCube.MipLevels           = static_cast<UINT>(-1);  // Use all mips.

        D3D12_SHADER_RESOURCE_VIEW_DESC IBLSRVDesc = {};
        IBLSRVDesc.Format                        = IBLDesc.Format;
        IBLSRVDesc.ViewDimension                 = D3D12_SRV_DIMENSION_TEXTURE2D;
        IBLSRVDesc.Texture2D.MostDetailedMip     = 0;
        IBLSRVDesc.Texture2D.MipLevels           = IBLDesc.MipLevels;
        IBLSRVDesc.Texture2D.PlaneSlice          = 0;
        IBLSRVDesc.Texture2D.ResourceMinLODClamp = 0;
        IBLSRVDesc.Shader4ComponentMapping       = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

        m_SkyboxSRV = Renderer::GetInstance().GetDevice()->CreateShaderResourceView(m_SkyboxCubemap, &cubeMapSRVDesc);
        m_SkyboxIrrSRV = Renderer::GetInstance().GetDevice()->CreateShaderResourceView(m_SkyboxIrrCubemap, &cubeMapSRVDesc);
        m_IBLTextureSRV = Renderer::GetInstance().GetDevice()->CreateShaderResourceView(m_IBLTexture, &IBLSRVDesc);

        spdlog::info("\tSetting up passes...");
        m_SkyboxPass = std::make_unique<SkyboxPass>(m_SceneMsaaRenderTarget, m_SkyboxSRV);
        m_GroundGridPass = std::make_unique<GroundGridPass>(m_SceneMsaaRenderTarget);
        m_ForwardOpaquePass = std::make_unique<ForwardOpaquePass>(m_SceneMsaaRenderTarget, m_SkyboxSRV, m_SkyboxIrrSRV, m_IBLTextureSRV);
        m_BloomPass = std::make_unique<BloomPass>(m_SceneMsaaRenderTarget, m_SceneHDRFrameBuffer);
        m_ToneMappingPass = std::make_unique<ToneMappingPass>(m_SceneSDRRenderTarget, m_SceneHDRFrameBuffer);
    }

    void ForwardPipeline::Render(const RenderContext& context)
    {
        {
            const auto cmd = Renderer::GetInstance().GetCommandListDirect();
            constexpr FLOAT clearColor[] = {0.2f, 0.2f, 0.2f, 1.0f};
            cmd->SetRenderTarget(*m_SceneMsaaRenderTarget);
            cmd->SetViewport(m_SceneMsaaRenderTarget->GetViewport());
            cmd->SetScissorRect(CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX));
            cmd->ClearTexture(m_SceneMsaaRenderTarget->GetTexture(Color0), clearColor);
            cmd->ClearDepthStencilTexture(m_SceneMsaaRenderTarget->GetTexture(DepthStencil), D3D12_CLEAR_FLAG_DEPTH, 0);
            Renderer::GetInstance().ExecuteCommandList(cmd);
        }

        m_SkyboxPass->Record(context);
        m_GroundGridPass->Record(context);
        m_ForwardOpaquePass->Record(context);
        
        m_SkyboxPass->Execute();
        m_GroundGridPass->Execute();
        m_ForwardOpaquePass->Execute();

        {
            const auto cmd = Renderer::GetInstance().GetCommandListDirect();
            cmd->ResolveSubresource(m_SceneHDRRenderTarget->GetTexture(Color0),
                                    m_SceneMsaaRenderTarget->GetTexture(Color0));
            Renderer::GetInstance().ExecuteCommandList(cmd);
        }

        m_BloomPass->Record(context);
        m_ToneMappingPass->Record(context);
        
        m_BloomPass->Execute();
        m_ToneMappingPass->Execute();
    }
}
