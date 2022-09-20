#pragma once

#include "Layer.h"

#include <imgui.h>

namespace Akari {
    class CommandList;
    class Device;
    class PipelineStateObject;
    class RenderTarget;
    class RootSignature;
    class ShaderResourceView;
    class Texture;

    class ImGuiLayer : public Layer
    {
    public:
        explicit ImGuiLayer(const std::string& name = "ImGuiLayer");
        ~ImGuiLayer() override;
        
        void OnAttach() override;
        void OnDetach() override;
        void OnUpdate(DeltaTime deltaTime) override;
        void OnEvent(Event& event) override;

    private:
        // Ref<RenderCommandBuffer> m_RenderCommandBuffer;
        float m_Time = 0.0f;

        HWND                                 m_hWnd;
        ImGuiContext*                        m_pImGuiCtx;
        std::shared_ptr<Device>              m_Device;
        std::shared_ptr<Texture>             m_FontTexture;
        std::shared_ptr<ShaderResourceView>  m_FontSRV;
        std::shared_ptr<RootSignature>       m_RootSignature;
        std::shared_ptr<PipelineStateObject> m_PipelineState;

        void Draw();
    };



}
