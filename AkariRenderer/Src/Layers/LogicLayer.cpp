#include "pch.h"
#include "LogicLayer.h"

#include "RHI/Renderer.h"
#include "RHI/RenderTarget.h"
#include "RPI/RenderContext.h"
#include "RPI/RenderPipeline.h"
#include "SceneComponents/Scene.h"
#include "SceneComponents/Camera/PerspectiveCamera.h"

namespace Akari
{
    LogicLayer::LogicLayer(const std::string& name)
    {
    }

    LogicLayer::~LogicLayer()
    {
    }

    void LogicLayer::OnAttach()
    {
        
    }

    void LogicLayer::OnDetach()
    {
    }

    void LogicLayer::OnUpdate(RenderContext& context)
    {
        const auto& cam = context.scene->GetCamera();
        const auto& rt = Renderer::GetInstance().GetRenderPipeline()->GetSceneRenderTarget();
        const float height = static_cast<float>(rt->GetHeight()), width = static_cast<float>(rt->GetWidth());
        cam->SetAspectRatio(height / width);
        cam->Update();
    }

    void LogicLayer::OnEvent(Event& event)
    {
    }
}
