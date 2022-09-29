#include "pch.h"
#include "LogicLayer.h"

#include "Input/Input.h"
#include "RHI/Renderer.h"
#include "RHI/RenderTarget.h"
#include "RPI/RenderContext.h"
#include "RPI/RenderPipeline.h"
#include "SceneComponents/Scene.h"
#include "SceneComponents/Camera/EditorCamera.h"

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
        const auto& rt = Renderer::GetInstance().GetRenderPipeline()->GetSceneSDRRenderTarget();
        const float height = rt->GetHeight(), width = rt->GetWidth();
        cam->SetViewportSize(width, height);
        cam->OnUpdate(*context.dt);
    }

    void LogicLayer::OnEvent(Event& event)
    {
    }
}
