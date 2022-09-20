#pragma once

#include "Layer.h"

namespace Akari {

    class ImGuiLayer : public Layer
    {
    public:
        ImGuiLayer();
        explicit ImGuiLayer(const std::string& name);
        ~ImGuiLayer() override;
        
        void OnAttach() override;
        void OnDetach() override;
        void OnUpdate(DeltaTime deltaTime) override;
        void OnEvent(Event& event) override;

    private:
        // Ref<RenderCommandBuffer> m_RenderCommandBuffer;
        float m_Time = 0.0f;
    };



}