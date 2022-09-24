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
        {
            const auto cmd = Renderer::GetInstance().GetCommandListDirect();
            constexpr FLOAT clearColor[] = { 0.8f, 0.6f, 0.2f, 1.0f };
            cmd->SetRenderTarget(*m_SceneRenderTarget);
            cmd->ClearTexture(m_SceneRenderTarget->GetTexture(Color0), clearColor);
            Renderer::GetInstance().ExecuteCommandList(cmd);
        }

        m_ForwardOpaquePass->Render(context);
    }
}
