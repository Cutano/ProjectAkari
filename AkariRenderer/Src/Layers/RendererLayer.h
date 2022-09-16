#pragma once

#include "Layer.h"

namespace Akari {

    class RendererLayer : public Layer
    {
    public:
        RendererLayer();
        explicit RendererLayer(const std::string& name);
        ~RendererLayer() override;
        
        void OnAttach() override;
        void OnDetach() override;
        void OnUpdate(DeltaTime deltaTime) override;
        void OnEvent(Event& event) override;

    private:
        // Ref<RenderCommandBuffer> m_RenderCommandBuffer;
        float m_Time = 0.0f;
    };



}
