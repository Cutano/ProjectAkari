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
        const auto& mainRt = Renderer::GetInstance().GetMsaaRenderTarget();
        const auto& device = Renderer::GetInstance().GetDevice();
        const DXGI_SAMPLE_DESC sampleDesc = Renderer::GetInstance().GetDevice()->GetMultisampleQualityLevels(DXGI_FORMAT_R11G11B10_FLOAT);
        
        const auto sceneMsaaFrameBufferDesc = CD3DX12_RESOURCE_DESC::Tex2D(
            DXGI_FORMAT_R11G11B10_FLOAT,
            mainRt->GetWidth(), mainRt->GetHeight(),
            1, 1,
            sampleDesc.Count, sampleDesc.Quality,
            D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
        auto sceneFrameBufferDesc = CD3DX12_RESOURCE_DESC::Tex2D(
            DXGI_FORMAT_R11G11B10_FLOAT,
            mainRt->GetWidth(), mainRt->GetHeight());
        sceneFrameBufferDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

        const auto sceneDepthStencilDesc = CD3DX12_RESOURCE_DESC::Tex2D(
            DXGI_FORMAT_D32_FLOAT,
            mainRt->GetWidth(), mainRt->GetHeight(),
            1, 1,
            sampleDesc.Count, sampleDesc.Quality,
            D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
        
        const D3D12_CLEAR_VALUE sceneClearValue {sceneMsaaFrameBufferDesc.Format, {0.2f, 0.2f, 0.2f, 1.0f}};
        D3D12_CLEAR_VALUE depthClearValue;
        depthClearValue.Format = sceneDepthStencilDesc.Format;
        depthClearValue.DepthStencil = {0.0f, 0};
        
        m_SceneFrameBuffer = device->CreateTexture(sceneFrameBufferDesc, &sceneClearValue);
        m_SceneFrameBuffer->SetName(L"SceneFrameBuffer");
        m_SceneMsaaFrameBuffer = device->CreateTexture(sceneMsaaFrameBufferDesc, &sceneClearValue);
        m_SceneMsaaFrameBuffer->SetName(L"SceneMSAAFrameBuffer");
        m_SceneDepth = device->CreateTexture(sceneDepthStencilDesc, &depthClearValue);
        m_SceneDepth->SetName(L"SceneDepth");

        m_SceneRenderTarget = std::make_shared<RenderTarget>();
        m_SceneMsaaRenderTarget = std::make_shared<RenderTarget>();
        m_SceneMsaaRenderTarget->AttachTexture(Color0, m_SceneMsaaFrameBuffer);
        m_SceneMsaaRenderTarget->AttachTexture(DepthStencil, m_SceneDepth);
        m_SceneRenderTarget->AttachTexture(Color0, m_SceneFrameBuffer);
    }

    RenderPipeline::~RenderPipeline()
    {
        m_SceneMsaaRenderTarget.reset();
        m_SceneRenderTarget.reset();
        m_SceneDepth.reset();
        m_SceneFrameBuffer.reset();
        m_SceneMsaaFrameBuffer.reset();
    }

    void RenderPipeline::OnEvent(Event& event)
    {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<SceneWindowResizeEvent>([this](SceneWindowResizeEvent& e) { return OnSceneResize(e); });
    }

    bool RenderPipeline::OnSceneResize(SceneWindowResizeEvent& event) const
    {
        m_SceneRenderTarget->Resize(static_cast<uint32_t>(event.GetWidth()), static_cast<uint32_t>(event.GetHeight()));
        m_SceneMsaaRenderTarget->Resize(static_cast<uint32_t>(event.GetWidth()), static_cast<uint32_t>(event.GetHeight()));

        spdlog::trace("Scene resized to {0}, {1}.", event.GetWidth(), event.GetHeight());
        return true;
    }

    std::shared_ptr<RenderTarget> RenderPipeline::GetSceneRenderTarget() const
    {
        return m_SceneRenderTarget;
    }
}
