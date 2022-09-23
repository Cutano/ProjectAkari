#include "pch.h"
#include "ForwardPipeline.h"
#include "Pass/ForwardOpaquePass.h"

#include "Layers/Layer.h"
#include "RHI/Renderer.h"
#include "RHI/SwapChain.h"
#include "RHI/CommandList.h"
#include "RPI/RenderContext.h"

namespace Akari
{
    ForwardPipeline::ForwardPipeline()
    {
        m_ForwardOpaquePass = std::make_unique<ForwardOpaquePass>();
    }

    void ForwardPipeline::Render(const RenderContext& context)
    {
        // TODO: Move this to Renderer
        {
            const auto cmd = Renderer::GetInstance().GetCommandListDirect();
            constexpr FLOAT clearColor[] = { 0.4f, 0.6f, 0.9f, 1.0f };
            cmd->SetRenderTarget(Renderer::GetInstance().GetSwapChain()->GetRenderTarget());
            cmd->ClearTexture(Renderer::GetInstance().GetSwapChain()->GetRenderTarget().GetTexture(Color0), clearColor);
            Renderer::GetInstance().ExecuteCommandList(cmd);
        }

        {
            const auto cmd = Renderer::GetInstance().GetCommandListDirect();
            constexpr FLOAT clearColor[] = { 0.8f, 0.6f, 0.2f, 1.0f };
            cmd->SetRenderTarget(*Renderer::GetInstance().GetSceneRenderTarget());
            cmd->ClearTexture(Renderer::GetInstance().GetSceneRenderTarget()->GetTexture(Color0), clearColor);
            Renderer::GetInstance().ExecuteCommandList(cmd);
        }

        m_ForwardOpaquePass->Render(context);

        if (m_GuiLayer != nullptr)
        {
            m_GuiLayer->OnUpdate(*context.dt);
        }
        else
        {
            spdlog::warn("No GUI Layer found, rendering without GUI!");
        }

        Renderer::GetInstance().GetSwapChain()->Present();
    }
}
