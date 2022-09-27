#pragma once

#include "Timing/DeltaTime.h"
#include "Events/Event.h"

namespace Akari {

    struct RenderContext;
    
    class Layer
    {
    public:
        Layer(const std::string& debugName = "Layer");
        virtual ~Layer();

        virtual void OnAttach() = 0;
        virtual void OnDetach() = 0;
        virtual void OnUpdate(RenderContext& context) = 0;
        virtual void OnEvent(Event& event) = 0;

        void SetEventCallback(const std::function<void(Event&)>& callback) { m_EventCallBack = callback; }

        const std::string& GetName() const { return m_DebugName; }
        
    protected:
        std::string m_DebugName;
        std::function<void(Event&)> m_EventCallBack;
    };

}