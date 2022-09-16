#pragma once

namespace Akari
{
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

        D3D12_VIEWPORT m_Viewport;
        D3D12_RECT m_ScissorRect;
        ComPtr<ID3D12Device9> m_Device;
        ComPtr<IDXGISwapChain4> m_SwapChain;
        ComPtr<ID3D12Resource2> m_RenderTargets[FrameCount];
        ComPtr<ID3D12CommandQueue> m_CommandQueueDirect;
        ComPtr<ID3D12DescriptorHeap> m_RtvHeap;

        UINT m_FrameIndex;
        UINT m_RtvDescriptorSize;
    };
    
}
