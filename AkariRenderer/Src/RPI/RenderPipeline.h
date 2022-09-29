#pragma once

namespace Akari
{
    struct RenderContext;
    class SceneWindowResizeEvent;
    class ShaderResourceView;
    class RenderTarget;
    class Texture;
    class Event;

    // Render pipelines focus on rendering scene window only.
    class RenderPipeline
    {
    public:
        RenderPipeline();
        virtual ~RenderPipeline();
        
        virtual void Render(const RenderContext& context) = 0;
        virtual void OnEvent(Event& event);
        virtual bool OnSceneResize(SceneWindowResizeEvent& event) const;
        
        [[nodiscard]] virtual std::shared_ptr<RenderTarget> GetSceneRenderTarget() const;

    protected:
        std::shared_ptr<Texture> m_SceneHDRFrameBuffer = nullptr;
        std::shared_ptr<Texture> m_SceneFrameBuffer = nullptr;
        std::shared_ptr<Texture> m_SceneDepth = nullptr;
        std::shared_ptr<Texture> m_SceneMsaaFrameBuffer = nullptr;
        
        // MSAA + HDR Texture
        std::shared_ptr<RenderTarget> m_SceneMsaaRenderTarget = nullptr;
        // HDR Texture
        std::shared_ptr<RenderTarget> m_SceneHDRRenderTarget = nullptr;
        // SDR Texture
        std::shared_ptr<RenderTarget> m_SceneRenderTarget = nullptr;
    };
}
