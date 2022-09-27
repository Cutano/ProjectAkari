#include "pch.h"
#include "LogicLayer.h"

#include "RPI/RenderContext.h"
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
        context.scene->GetCamera()->Update();
    }

    void LogicLayer::OnEvent(Event& event)
    {
    }
}
