#include "pch.h"
#include "ForwardPipeline.h"

#include "Layers/Layer.h"
#include "RHI/Renderer.h"
#include "RHI/SwapChain.h"
#include "RHI/CommandList.h"
#include "RPI/RenderContext.h"

namespace Akari
{
    ForwardPipeline::ForwardPipeline()
    {

    }

    void ForwardPipeline::Render(const RenderContext& context)
    {
        const auto cmd = Renderer::GetInstance().GetCommandListDirect();
        constexpr FLOAT clearColor[] = { 0.4f, 0.6f, 0.9f, 1.0f };
        cmd->ClearTexture(Renderer::GetInstance().GetSwapChain()->GetRenderTarget().GetTexture(Color0), clearColor);
        Renderer::GetInstance().ExecuteCommandList(cmd);

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