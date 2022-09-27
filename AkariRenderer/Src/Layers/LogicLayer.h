#pragma once

#include "Layer.h"

namespace Akari {

    class LogicLayer : public Layer
    {
    public:
        explicit LogicLayer(const std::string& name = "LogicLayer");
        ~LogicLayer() override;
        
        void OnAttach() override;
        void OnDetach() override;
        void OnUpdate(RenderContext& context) override;
        void OnEvent(Event& event) override;

    private:
        // Ref<RenderCommandBuffer> m_RenderCommandBuffer;
        float m_Time = 0.0f;
    };



}
