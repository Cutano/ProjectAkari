#include "pch.h"
#include "ForwardPipeline.h"
#include "Pass/ForwardOpaquePass.h"
#include "Pass/GroundGridPass.h"

#include "RHI/Renderer.h"
#include "RHI/SwapChain.h"
#include "RHI/Texture.h"
#include "RHI/CommandList.h"
#include "RPI/RenderContext.h"

namespace Akari
{
    ForwardPipeline::ForwardPipeline()
    {
        m_SkyboxPass = std::make_unique<SkyboxPass>(m_SceneMsaaRenderTarget);
        m_GroundGridPass = std::make_unique<GroundGridPass>(m_SceneMsaaRenderTarget);
        m_ForwardOpaquePass = std::make_unique<ForwardOpaquePass>(m_SceneMsaaRenderTarget);
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
            cmd->ResolveSubresource(m_SceneRenderTarget->GetTexture(Color0),
                                    m_SceneMsaaRenderTarget->GetTexture(Color0));
            Renderer::GetInstance().ExecuteCommandList(cmd);
        }
    }
}
