#include "pch.h"
#include "RenderPipeline.h"

#include "RHI/Renderer.h"

namespace Akari
{
    RenderPipeline::RenderPipeline()
    {

    }

    RenderPipeline::RenderPipeline(std::shared_ptr<Layer>& guiLayer)
    {
        m_GuiLayer = guiLayer;
    }

    RenderPipeline::~RenderPipeline()
    {
        m_GuiLayer.reset();
    }
}
