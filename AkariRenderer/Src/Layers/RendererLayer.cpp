#include "pch.h"
#include "RendererLayer.h"

#include "RHI/Renderer.h"

namespace Akari
{
    RendererLayer::RendererLayer()
    {
    }

    RendererLayer::RendererLayer(const std::string& name)
    {
    }

    RendererLayer::~RendererLayer()
    {
    }

    void RendererLayer::OnAttach()
    {
        Renderer::GetInstance().Init();
    }

    void RendererLayer::OnDetach()
    {
        Renderer::GetInstance().ShutDown();
    }

    void RendererLayer::OnUpdate(DeltaTime deltaTime)
    {
    }

    void RendererLayer::OnEvent(Event& event)
    {
    }
}
