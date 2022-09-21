#pragma once

namespace Akari
{
    struct RenderContext;
    class Renderer;
    class Layer;
    
    class RenderPipeline
    {
    public:
        RenderPipeline();
        explicit RenderPipeline(std::shared_ptr<Layer>& guiLayer);
        virtual ~RenderPipeline();
        
        virtual void Render(const RenderContext& context) = 0;
        virtual void SetGuiLayer(std::shared_ptr<Layer>& layer) {m_GuiLayer = layer;};

    protected:
        std::shared_ptr<Layer> m_GuiLayer;
    };
}
