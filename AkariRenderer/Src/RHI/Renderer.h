#pragma once

namespace Akari
{
    class Device;
    class SwapChain;
    
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

    private:
        Renderer() = default;

        std::shared_ptr<Device> m_Device = nullptr;
        std::shared_ptr<SwapChain> m_SwapChain = nullptr;
    };
    
}
