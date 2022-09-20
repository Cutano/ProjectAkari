#pragma once
#include "Timing/DeltaTime.h"

namespace Akari
{
    class Device;
    class SwapChain;
    class CommandList;
    
    class Renderer
    {
    public:
        static constexpr uint8_t FrameCount = 2;
        
        static Renderer& GetInstance()
        {
            static Renderer s_Instance;
            return s_Instance;
        }
        
        Renderer(Renderer const&)       = delete;
        void operator=(Renderer const&) = delete;

        void Init();
        void ShutDown();
        
        void OnUpdate(DeltaTime dt);
        void OnResize() const;

        [[nodiscard]] std::shared_ptr<CommandList> getCommandListDirect() const;
        [[nodiscard]] std::shared_ptr<CommandList> getCommandListCopy() const;
        [[nodiscard]] std::shared_ptr<CommandList> getCommandListCompute() const;

        void ExecuteCommandList(std::shared_ptr<CommandList> commandList) const;

    private:
        Renderer() = default;

        std::shared_ptr<Device> m_Device = nullptr;
        std::shared_ptr<SwapChain> m_SwapChain = nullptr;
    };
    
}
