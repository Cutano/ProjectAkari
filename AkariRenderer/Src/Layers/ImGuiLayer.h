#pragma once

#include <future>

#include "Layer.h"

#include <imgui.h>

#include "UUID.h"
#include "SceneComponents/Visitor.h"

namespace Akari {
    class KeyPressedEvent;
    class CommandList;
    class Device;
    class PipelineStateObject;
    class RenderTarget;
    class RootSignature;
    class ShaderResourceView;
    class Texture;
    class SceneWindowResizeEvent;
    class SceneObject;

    class ImGuiLayer : public Layer
    {
    public:
        explicit ImGuiLayer(const std::string& name = "ImGuiLayer");
        ~ImGuiLayer() override;
        
        void OnAttach() override;
        void OnDetach() override;
        void OnUpdate(RenderContext& context) override;
        void OnEvent(Event& event) override;

        bool m_IsSceneWindowHovered;

    private:
        // Ref<RenderCommandBuffer> m_RenderCommandBuffer;
        float m_Time = 0.0f;
        bool m_ShowDemoWindow = true;
        bool m_ShowSceneWindow = true;
        bool m_ShowHierarchyWindow = true;
        bool m_ShowBrowserWindow = true;
        bool m_ShowPropertyWindow = true;
        bool m_ShowToneMappingSettings = false;
        bool m_ShowBloomSettings = false;

        float m_SceneWindowWidth;
        float m_SceneWindowHeight;
        float m_SceneWindowPosX;
        float m_SceneWindowPosY;

        int m_GizmoType = -1; // -1 = no gizmo
        int m_GizmoMode = 0; // 0 = local

        UUID m_SelectedSceneObject{0};

        ImGuiContext*                        m_pImGuiCtx = nullptr;
        HWND                                 m_hWnd;
        std::shared_ptr<Device>              m_Device;
        std::shared_ptr<Texture>             m_FontTexture;
        std::shared_ptr<ShaderResourceView>  m_FontSRV;
        std::shared_ptr<RootSignature>       m_RootSignature;
        std::shared_ptr<PipelineStateObject> m_PipelineState;

        std::future<bool> m_LoadingTask;

        std::atomic_bool m_IsLoading;
        std::atomic_bool m_CancelLoading;;
        float            m_LoadingProgress;
        std::string      m_LoadingText;

        std::unordered_map<std::wstring, UUID> m_LoadedModelList;

        void Draw();
        void DrawSceneWindow();
        void DrawHierarchyWindow();
        void DrawBrowserWindow();
        void DrawPropertyWindow();
        void DrawToneMappingSettingsWindow();
        void DrawBloomSettingsWindow();

        void DrawHierarchyNode(SceneObject& obj);
        void DrawGizmo();
        void SetStyle();

        void OpenLoadModelDialog();
        bool LoadModel(std::wstring path);

        bool OnKeyPressedEvent(KeyPressedEvent& e);

        static void ShowHelpMarker(const char* desc);
        static float LinearTonemapping(float HDR, float max);
        static float LinearTonemappingPlot(void*, int index);
        static float ReinhardTonemapping(float HDR, float k);
        static float ReinhardTonemappingPlot(void*, int index);
        static float ReinhardSqrTonemappingPlot(void*, int index);
        static float ACESFilmicTonemapping(float x, float A, float B, float C, float D, float E, float F);
        static float ACESFilmicTonemappingPlot(void*, int index);
    };

    class ModelVisitor final : public Visitor
    {
        void Visit(Scene& scene) override;
        void Visit(SceneObject& model) override;
        void Visit(ModelNode& modelNode) override;
        void Visit(Mesh& mesh) override;
    };

}
