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
    class SceneWindowResizeEvent;

    class ImGuiLayer : public Layer
    {
    public:
        explicit ImGuiLayer(const std::string& name = "ImGuiLayer");
        ~ImGuiLayer() override;
        
        void OnAttach() override;
        void OnDetach() override;
        void OnUpdate(RenderContext& context) override;
        void OnEvent(Event& event) override;

    private:
        // Ref<RenderCommandBuffer> m_RenderCommandBuffer;
        float m_Time = 0.0f;
        bool m_ShowDemoWindow = true;
        bool m_ShowSceneWindow = true;
        bool m_ShowHierarchyWindow = true;
        bool m_ShowBrowserWindow = true;
        bool m_ShowPropertyWindow = true;

        float m_SceneWindowWidth;
        float m_SceneWindowHeight;

        ImGuiContext*                        m_pImGuiCtx = nullptr;
        HWND                                 m_hWnd;
        std::shared_ptr<Device>              m_Device;
        std::shared_ptr<Texture>             m_FontTexture;
        std::shared_ptr<ShaderResourceView>  m_FontSRV;
        std::shared_ptr<RootSignature>       m_RootSignature;
        std::shared_ptr<PipelineStateObject> m_PipelineState;

        void Draw();
        void DrawSceneWindow();
        void DrawHierarchyWindow();
        void DrawBrowserWindow();
        void DrawPropertyWindow();

        void SetStyle();
    };



}
