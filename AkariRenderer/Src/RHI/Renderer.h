#pragma once
#include "Timing/DeltaTime.h"

namespace Akari
{
    class Device;
    class Texture;
    class RenderTarget;
    class SwapChain;
    class CommandList;
    struct RenderContext;
    
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
        void LoadModel(std::wstring path);
        
        void OnUpdate(RenderContext& context);
        void OnResize(uint32_t width, uint32_t height) const;
        void OnSceneResize(float width, float height) const;

        [[nodiscard]] std::shared_ptr<CommandList> GetCommandListDirect() const;
        [[nodiscard]] std::shared_ptr<CommandList> GetCommandListCopy() const;
        [[nodiscard]] std::shared_ptr<CommandList> GetCommandListCompute() const;

        [[nodiscard]] std::shared_ptr<Device> GetDevice() const;
        [[nodiscard]] std::shared_ptr<SwapChain> GetSwapChain() const;
        [[nodiscard]] std::shared_ptr<RenderTarget> GetSceneRenderTarget() const;

        uint64_t ExecuteCommandList(std::shared_ptr<CommandList> commandList) const;

    private:
        Renderer() = default;

        std::shared_ptr<Device> m_Device = nullptr;
        std::shared_ptr<SwapChain> m_SwapChain = nullptr;

        // TODO: Move all of this to GUILayer
        std::shared_ptr<Texture> m_SceneFrameBuffer = nullptr;
        std::shared_ptr<Texture> m_SceneDepth = nullptr;
        std::shared_ptr<RenderTarget> m_SceneRenderTarget = nullptr;
    };
    
}
