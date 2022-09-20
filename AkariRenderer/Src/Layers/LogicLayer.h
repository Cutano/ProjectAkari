#pragma once

#include "Layer.h"

namespace Akari {

    class LogicLayer : public Layer
    {
    public:
        LogicLayer();
        explicit LogicLayer(const std::string& name);
        ~LogicLayer() override;
        
        void OnAttach() override;
        void OnDetach() override;
        void OnUpdate(DeltaTime deltaTime) override;
        void OnEvent(Event& event) override;

    private:
        // Ref<RenderCommandBuffer> m_RenderCommandBuffer;
        float m_Time = 0.0f;
    };



}
