#pragma once
#include "Timing/DeltaTime.h"

namespace Akari
{
    class Device;
    class Texture;
    class Model;
    class RenderTarget;
    class SwapChain;
    class CommandList;
    class ShaderResourceView;
    class ImGuiLayer;
    class RenderPipeline;
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
        std::shared_ptr<Model> LoadModel(std::wstring path);
        
        void OnUpdate(RenderContext& context);
        void OnResize(uint32_t width, uint32_t height) const;

        [[nodiscard]] std::shared_ptr<CommandList> GetCommandListDirect() const;
        [[nodiscard]] std::shared_ptr<CommandList> GetCommandListCopy() const;
        [[nodiscard]] std::shared_ptr<CommandList> GetCommandListCompute() const;

        [[nodiscard]] std::shared_ptr<Device> GetDevice() const;
        [[nodiscard]] std::shared_ptr<SwapChain> GetSwapChain() const;
        [[nodiscard]] std::shared_ptr<RenderTarget> GetMsaaRenderTarget() const;
        [[nodiscard]] std::shared_ptr<ImGuiLayer> GetImGuiLayer() const;
        [[nodiscard]] std::shared_ptr<RenderPipeline> GetRenderPipeline() const;

        void SetRenderPipeline(const std::shared_ptr<RenderPipeline>& rp);

        uint64_t ExecuteCommandList(std::shared_ptr<CommandList> commandList) const;

    private:
        Renderer() = default;

        std::shared_ptr<Device> m_Device = nullptr;
        std::shared_ptr<SwapChain> m_SwapChain = nullptr;
        std::shared_ptr<ImGuiLayer> m_ImGuiLayer = nullptr;
        std::shared_ptr<RenderPipeline> m_RenderPipeline = nullptr;
        std::shared_ptr<RenderTarget> m_MsaaRenderTarget = nullptr;
    };
    
}
