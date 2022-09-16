#pragma once

#include "Timing/DeltaTime.h"
#include "Events/Event.h"

namespace Akari {

    class Layer
    {
    public:
        Layer(const std::string& debugName = "Layer");
        virtual ~Layer();

        virtual void OnAttach() = 0;
        virtual void OnDetach() = 0;
        virtual void OnUpdate(DeltaTime deltaTime) = 0;
        virtual void OnEvent(Event& event) = 0;

        const std::string& GetName() const { return m_DebugName; }
    protected:
        std::string m_DebugName;
    };

}