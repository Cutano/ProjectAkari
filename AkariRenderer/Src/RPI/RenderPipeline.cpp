#include "pch.h"
#include "RenderPipeline.h"

#include "Events/ApplicationEvent.h"
#include "Events/Event.h"
#include "RHI/Renderer.h"
#include "RHI/SwapChain.h"
#include "RHI/Device.h"
#include "RHI/Texture.h"

namespace Akari
{
    RenderPipeline::RenderPipeline()
    {
        const auto& mainRt = Renderer::GetInstance().GetSwapChain()->GetRenderTarget();
        const auto& device = Renderer::GetInstance().GetDevice();
        auto sceneFrameBufferDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R11G11B10_FLOAT, mainRt.GetWidth(), mainRt.GetHeight());
        auto sceneDepthStencilDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, mainRt.GetWidth(), mainRt.GetHeight());
        sceneFrameBufferDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
        sceneDepthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
        m_SceneFrameBuffer = device->CreateTexture(sceneFrameBufferDesc);
        m_SceneDepth = device->CreateTexture(sceneDepthStencilDesc);
        m_SceneFrameBufferSRV = device->CreateShaderResourceView(m_SceneFrameBuffer);

        m_SceneRenderTarget = std::make_shared<RenderTarget>();
        m_SceneRenderTarget->AttachTexture(Color0, m_SceneFrameBuffer);
        m_SceneRenderTarget->AttachTexture(DepthStencil, m_SceneDepth);
    }

    RenderPipeline::~RenderPipeline()
    {
        m_SceneRenderTarget.reset();
        m_SceneFrameBufferSRV.reset();
        m_SceneDepth.reset();
        m_SceneFrameBuffer.reset();
    }

    void RenderPipeline::OnEvent(Event& event)
    {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<SceneWindowResizeEvent>([this](SceneWindowResizeEvent& e) { return OnSceneResize(e); });
    }

    bool RenderPipeline::OnSceneResize(SceneWindowResizeEvent& event) const
    {
        m_SceneRenderTarget->Resize(static_cast<uint32_t>(event.GetWidth()), static_cast<uint32_t>(event.GetHeight()));

        spdlog::info("Scene resized to {0}, {1}.", event.GetWidth(), event.GetHeight());
        return true;
    }

    std::shared_ptr<ShaderResourceView> RenderPipeline::GetSceneFrameBufferSrv() const
    {
        return m_SceneFrameBufferSRV;
    }

    std::shared_ptr<RenderTarget> RenderPipeline::GetSceneRenderTarget() const
    {
        return m_SceneRenderTarget;
    }
}
