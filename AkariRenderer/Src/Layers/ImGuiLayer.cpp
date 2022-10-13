#include "pch.h"
#include "ImGuiLayer.h"
#include "Application/Application.h"
#include "RHI/Renderer.h"
#include "RHI/Device.h"
#include "RHI/Texture.h"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_dx12.h>

#include <ImGuizmo.h>

#include "Events/KeyEvent.h"
#include "Input/Input.h"
#include "RHI/DynamicDescriptorHeap.h"
#include "RHI/CommandQueue.h"
#include "RHI/CommandList.h"
#include "RHI/RootSignature.h"
#include "RHI/ShaderResourceView.h"
#include "RHI/SwapChain.h"
#include "RPI/RenderPipeline.h"
#include "Window/WindowsWindow.h"
#include "SceneComponents/Scene.h"
#include "SceneComponents/SceneObject.h"

#include "Shaders/Generated/ImGUI_PS.h"
#include "Shaders/Generated/ImGUI_VS.h"

#include "RenderPipelines/Pass/ToneMappingPass/ToneMappingParameters.h"
#include "SceneComponents/Material.h"
#include "SceneComponents/Mesh.h"
#include "SceneComponents/Model.h"
#include "SceneComponents/ModelManager.h"
#include "SceneComponents/ModelNode.h"
#include "SceneComponents/Scene.h"
#include "SceneComponents/Camera/EditorCamera.h"

//--------------------------------------------------------------------------------------
// Get surface information for a particular format
//--------------------------------------------------------------------------------------
void GetSurfaceInfo(_In_ size_t width, _In_ size_t height, _In_ DXGI_FORMAT fmt, size_t* outNumBytes,
                    _Out_opt_ size_t* outRowBytes, _Out_opt_ size_t* outNumRows);

namespace Akari
{
    // Number of values to plot in the tonemapping curves.
    static const int VALUES_COUNT = 512;
    // Maximum HDR value to normalize the plot samples.
    static const float HDR_MAX = 12.0f;
    
    // Root parameters for the ImGui root signature.
    enum ImGuiRootParameters
    {
        // cbuffer vertexBuffer : register(b0)
        MatrixCB,
        // cbuffer TexIdBuffer : register(b1)
        TexIdCB,
        // Texture2D textureX : register(tX);
        UITextures,
        NumRootParameters
    };

    enum UITextureStack
    {
        FontTexture,
        SceneTexture,
        NumUITexture
    };

    ImGuiLayer::ImGuiLayer(const std::string& name)
    {
        m_Device = Renderer::GetInstance().GetDevice();
        m_hWnd = dynamic_cast<WindowsWindow*>(&Application::Get().GetWindow())->GetHandle();

        m_SceneWindowWidth = static_cast<float>(Renderer::GetInstance().GetSwapChain()->GetRenderTarget().GetWidth());
        m_SceneWindowHeight = static_cast<float>(Renderer::GetInstance().GetSwapChain()->GetRenderTarget().GetHeight());
    }

    ImGuiLayer::~ImGuiLayer()
    {
    }

    void ImGuiLayer::OnAttach()
    {
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        m_pImGuiCtx = ImGui::CreateContext();
        ImGui::SetCurrentContext(m_pImGuiCtx);
        ImGuiIO& io = ImGui::GetIO();
        (void)io;
        io.FontGlobalScale = static_cast<float>(GetDpiForWindow(m_hWnd)) / 96.0f;
        // Allow user UI scaling using CTRL+Mouse Wheel scrolling
        io.FontAllowUserScaling = true;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
        // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows
        //io.ConfigViewportsNoDecoration = false;
        //io.ConfigViewportsNoAutoMerge = true;
        //io.ConfigViewportsNoTaskBarIcon = true;

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();

        io.Fonts->AddFontFromFileTTF("Res\\Fonts\\MiSans-Medium.ttf", 16.0f, nullptr,
                                     io.Fonts->GetGlyphRangesChineseFull());

        // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        SetStyle();

        ImGui_ImplGlfw_InitForOther(static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow()), true);

        // Build texture atlas
        unsigned char* pixelData = nullptr;
        int width, height;
        io.Fonts->GetTexDataAsRGBA32(&pixelData, &width, &height);
        io.Fonts->SetTexID(reinterpret_cast<ImTextureID>(FontTexture));

        auto& commandQueue = m_Device->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COPY);
        auto commandList = commandQueue.GetCommandList();

        auto fontTextureDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, width, height);

        m_FontTexture = m_Device->CreateTexture(fontTextureDesc);
        m_FontTexture->SetName(L"ImGui Font Texture");
        m_FontSRV = m_Device->CreateShaderResourceView(m_FontTexture);

        size_t rowPitch, slicePitch;
        GetSurfaceInfo(width, height, DXGI_FORMAT_R8G8B8A8_UNORM, &slicePitch, &rowPitch, nullptr);

        D3D12_SUBRESOURCE_DATA subresourceData;
        subresourceData.pData = pixelData;
        subresourceData.RowPitch = rowPitch;
        subresourceData.SlicePitch = slicePitch;

        commandList->CopyTextureSubresource(m_FontTexture, 0, 1, &subresourceData);
        commandList->GenerateMips(m_FontTexture);

        commandQueue.ExecuteCommandList(commandList);

        auto d3d12Device = m_Device->GetD3D12Device();

        // Create the root signature for the ImGUI shaders.

        // Allow input layout and deny unnecessary access to certain pipeline stages.
        D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

        CD3DX12_DESCRIPTOR_RANGE1 texDescriptorRage(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, NumUITexture, 0);

        CD3DX12_ROOT_PARAMETER1 rootParameters[NumRootParameters];
        rootParameters[MatrixCB].InitAsConstants(
            sizeof(DirectX::XMMATRIX) / 4, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX);
        rootParameters[TexIdCB].InitAsConstants(1, 1, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);
        rootParameters[UITextures].InitAsDescriptorTable(1, &texDescriptorRage, D3D12_SHADER_VISIBILITY_PIXEL);

        CD3DX12_STATIC_SAMPLER_DESC linearRepeatSampler(0, D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT);
        linearRepeatSampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
        linearRepeatSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
        rootSignatureDescription.Init_1_1(ImGuiRootParameters::NumRootParameters, rootParameters, 1, &linearRepeatSampler,
                                          rootSignatureFlags);

        m_RootSignature = m_Device->CreateRootSignature(rootSignatureDescription.Desc_1_1);

        // clang-format off
        const D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
            {
                "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(ImDrawVert, pos),
                D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
            },
            {
                "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(ImDrawVert, uv),
                D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
            },
            {
                "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, offsetof(ImDrawVert, col),
                D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
            },
        };
        // clang-format on

        D3D12_BLEND_DESC blendDesc = {};
        blendDesc.RenderTarget[0].BlendEnable = true;
        blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
        blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
        blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
        blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

        D3D12_RASTERIZER_DESC rasterizerDesc = {};
        rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
        rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
        rasterizerDesc.FrontCounterClockwise = FALSE;
        rasterizerDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
        rasterizerDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
        rasterizerDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
        rasterizerDesc.DepthClipEnable = true;
        rasterizerDesc.MultisampleEnable = TRUE;
        rasterizerDesc.AntialiasedLineEnable = TRUE;
        rasterizerDesc.ForcedSampleCount = 0;
        rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

        D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {};
        depthStencilDesc.DepthEnable = false;
        depthStencilDesc.StencilEnable = false;

        const auto& renderTarget = Renderer::GetInstance().GetMsaaRenderTarget();

        // Setup the pipeline state.
        struct PipelineStateStream
        {
            CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
            CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
            CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
            CD3DX12_PIPELINE_STATE_STREAM_VS VS;
            CD3DX12_PIPELINE_STATE_STREAM_PS PS;
            CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
            CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC SampleDesc;
            CD3DX12_PIPELINE_STATE_STREAM_BLEND_DESC BlendDesc;
            CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER RasterizerState;
            CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL DepthStencilState;
        } pipelineStateStream;

        pipelineStateStream.pRootSignature = m_RootSignature->GetD3D12RootSignature().Get();
        pipelineStateStream.InputLayout = {inputLayout, 3};
        pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        pipelineStateStream.VS = {g_ImGUI_VS, sizeof g_ImGUI_VS};
        pipelineStateStream.PS = {g_ImGUI_PS, sizeof g_ImGUI_PS};
        pipelineStateStream.RTVFormats = renderTarget->GetRenderTargetFormats();
        pipelineStateStream.SampleDesc = renderTarget->GetSampleDesc();
        pipelineStateStream.BlendDesc = CD3DX12_BLEND_DESC(blendDesc);
        pipelineStateStream.RasterizerState = CD3DX12_RASTERIZER_DESC(rasterizerDesc);
        pipelineStateStream.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(depthStencilDesc);

        m_PipelineState = m_Device->CreatePipelineStateObject(pipelineStateStream);
    }

    void ImGuiLayer::OnDetach()
    {
        ImGui::EndFrame();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext(m_pImGuiCtx);
        m_pImGuiCtx = nullptr;
    }

    void ImGuiLayer::OnUpdate(RenderContext& context)
    {
        ImGui::SetCurrentContext(m_pImGuiCtx);
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGuizmo::BeginFrame();

        Draw();

        ImGui::Render();

        ImGuiIO& io = ImGui::GetIO();
        ImDrawData* drawData = ImGui::GetDrawData();

        // Check if there is anything to render.
        if (!drawData || drawData->CmdListsCount == 0)
            return;

        ImVec2 displayPos = drawData->DisplayPos;

        const auto& commandList = Renderer::GetInstance().GetCommandListDirect();
        const auto& renderTarget = Renderer::GetInstance().GetMsaaRenderTarget();

        commandList->SetPipelineState(m_PipelineState);
        commandList->SetGraphicsRootSignature(m_RootSignature);
        commandList->SetRenderTarget(*renderTarget);

        // Set root arguments.
        //    DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixOrthographicRH( drawData->DisplaySize.x,
        //    drawData->DisplaySize.y, 0.0f, 1.0f );
        float L = drawData->DisplayPos.x;
        float R = drawData->DisplayPos.x + drawData->DisplaySize.x;
        float T = drawData->DisplayPos.y;
        float B = drawData->DisplayPos.y + drawData->DisplaySize.y;
        float mvp[4][4] = {
            {2.0f / (R - L), 0.0f, 0.0f, 0.0f},
            {0.0f, 2.0f / (T - B), 0.0f, 0.0f},
            {0.0f, 0.0f, 0.5f, 0.0f},
            {(R + L) / (L - R), (T + B) / (B - T), 0.5f, 1.0f},
        };

        const auto& frameBuffer = Renderer::GetInstance().GetRenderPipeline()->GetSceneSDRRenderTarget()->
                                                          GetTexture(Color0);
        commandList->SetGraphics32BitConstants(MatrixCB, mvp);
        commandList->SetShaderResourceView(UITextures, FontTexture, m_FontSRV,
                                           D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        commandList->SetShaderResourceView(UITextures, SceneTexture, frameBuffer,
                                           D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

        D3D12_VIEWPORT viewport = {};
        viewport.Width = drawData->DisplaySize.x;
        viewport.Height = drawData->DisplaySize.y;
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;

        commandList->SetViewport(viewport);
        commandList->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        const DXGI_FORMAT indexFormat = sizeof(ImDrawIdx) == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;

        // It may happen that ImGui doesn't actually render anything. In this case,
        // any pending resource barriers in the commandList will not be flushed (since
        // resource barriers are only flushed when a draw command is executed).
        // In that case, manually flushing the resource barriers will ensure that
        // they are properly flushed before exiting this function.
        commandList->FlushResourceBarriers();

        for (int i = 0; i < drawData->CmdListsCount; ++i)
        {
            const ImDrawList* drawList = drawData->CmdLists[i];

            commandList->SetDynamicVertexBuffer(0, drawList->VtxBuffer.size(), sizeof(ImDrawVert),
                                                drawList->VtxBuffer.Data);
            commandList->SetDynamicIndexBuffer(drawList->IdxBuffer.size(), indexFormat, drawList->IdxBuffer.Data);

            int indexOffset = 0;
            for (int j = 0; j < drawList->CmdBuffer.size(); ++j)
            {
                const ImDrawCmd& drawCmd = drawList->CmdBuffer[j];
                if (drawCmd.UserCallback)
                {
                    drawCmd.UserCallback(drawList, &drawCmd);
                }
                else
                {
                    ImVec4 clipRect = drawCmd.ClipRect;
                    D3D12_RECT scissorRect;
                    scissorRect.left = static_cast<LONG>(clipRect.x - displayPos.x);
                    scissorRect.top = static_cast<LONG>(clipRect.y - displayPos.y);
                    scissorRect.right = static_cast<LONG>(clipRect.z - displayPos.x);
                    scissorRect.bottom = static_cast<LONG>(clipRect.w - displayPos.y);

                    if (scissorRect.right - scissorRect.left > 0.0f && scissorRect.bottom - scissorRect.top > 0.0)
                    {
                        auto texID = static_cast<uint32_t>(reinterpret_cast<uint64_t>(drawCmd.GetTexID()));
                        commandList->SetGraphics32BitConstants(TexIdCB, 1, &texID);
                        commandList->SetScissorRect(scissorRect);
                        commandList->DrawIndexed(drawCmd.ElemCount, 1, indexOffset);
                    }
                }
                indexOffset += drawCmd.ElemCount;
            }
        }

        Renderer::GetInstance().ExecuteCommandList(commandList);
    }

    void ImGuiLayer::OnEvent(Event& event)
    {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<KeyPressedEvent>([this](KeyPressedEvent& e) { return OnKeyPressedEvent(e); });
    }

    void ImGuiLayer::Draw()
    {
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove;
        windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        ImGui::Begin("DockSpaceWindow", nullptr, windowFlags);
        ImGui::PopStyleVar(3);
        const ImGuiID dockSpaceId = ImGui::GetID("DockSpace");
        ImGui::DockSpace(dockSpaceId, ImVec2(0.0f, 0.0f));

        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("Window"))
            {
                ImGui::MenuItem("Show Demo", nullptr, &m_ShowDemoWindow);
                ImGui::MenuItem("Show Scene", nullptr, &m_ShowSceneWindow);
                ImGui::MenuItem("Show Hierarchy", nullptr, &m_ShowHierarchyWindow);
                ImGui::MenuItem("Show Browser", nullptr, &m_ShowBrowserWindow);
                ImGui::MenuItem("Show Property", nullptr, &m_ShowPropertyWindow);
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Post Processing"))
            {
                ImGui::MenuItem("Tone Mapping", nullptr, &m_ShowToneMappingSettings);
                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }

        ImGui::End();

        if (m_ShowDemoWindow)
        {
            ImGui::ShowDemoWindow(&m_ShowDemoWindow);
        }
        if (m_ShowSceneWindow)
        {
            DrawSceneWindow();
        }
        if (m_ShowHierarchyWindow)
        {
            DrawHierarchyWindow();
        }
        if (m_ShowBrowserWindow)
        {
            DrawBrowserWindow();
        }
        if (m_ShowPropertyWindow)
        {
            DrawPropertyWindow();
        }
        if (m_ShowToneMappingSettings)
        {
            DrawToneMappingSettingsWindow();
        }
    }

    // https://github.com/ocornut/imgui/issues/984
    void ImGuiLayer::DrawSceneWindow()
    {
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar;
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Scene", &m_ShowSceneWindow, windowFlags);
        ImGui::PopStyleVar();
        
        m_IsSceneWindowHovered = ImGui::IsWindowHovered();

        ImVec2 view = ImGui::GetContentRegionAvail();
        if (view.x != m_SceneWindowWidth || view.y != m_SceneWindowHeight)
        {
            if (view.x == 0 || view.y == 0)
            {
                // The window is too small or collapsed.
                ImGui::End();
                return;
            }

            m_SceneWindowWidth = view.x;
            m_SceneWindowHeight = view.y;

            SceneWindowResizeEvent e(view.x, view.y);
            Renderer::GetInstance().GetRenderPipeline()->OnEvent(e);
        }

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        const auto textureId = reinterpret_cast<ImTextureID>(SceneTexture);
        ImVec2 vMin = ImGui::GetWindowContentRegionMin();
        ImVec2 vMax = ImGui::GetWindowContentRegionMax();
        vMin.x += ImGui::GetWindowPos().x;
        vMin.y += ImGui::GetWindowPos().y;
        vMax.x += ImGui::GetWindowPos().x;
        vMax.y += ImGui::GetWindowPos().y;
        drawList->AddImage(textureId, vMin, vMax);

        m_SceneWindowPosX = vMin.x;
        m_SceneWindowPosY = vMin.y;

        DrawGizmo();
        
        ImGui::End();
    }

    void ImGuiLayer::DrawHierarchyWindow()
    {
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar;
        ImGui::Begin("Hierarchy", &m_ShowHierarchyWindow, windowFlags);

        auto& scene = Application::Get().GetScene();

        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("Create"))
            {
                if (ImGui::MenuItem("Cube"))
                {
                    auto cube = scene.CreateSceneObject("Cube");
                    auto & [ModelID]= cube.AddComponent<ModelComponent>();
                    ModelID = ModelManager::GetInstance().GetCubeID();
                }

                if (ImGui::MenuItem("Sphere"))
                {
                    auto cube = scene.CreateSceneObject("Sphere");
                    auto & [ModelID]= cube.AddComponent<ModelComponent>();
                    ModelID = ModelManager::GetInstance().GetSphereID();
                }

                if (ImGui::MenuItem("Directional Light"))
                {
                    auto dirLight = scene.CreateSceneObject("Directional Light");
                    dirLight.AddComponent<DirectionalLightComponent>();
                }
                
                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }

        const auto entities = scene.GetAllSceneObjectsWith<IDComponent, RelationshipComponent>();
        for (const auto entity : entities)
        {
            SceneObject obj(entity, &scene);
            DrawHierarchyNode(obj);
        }
        
        ImGui::End();
    }

    void ImGuiLayer::DrawBrowserWindow()
    {
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar;
        ImGui::Begin("Browser", &m_ShowBrowserWindow, windowFlags);
        ImGui::End();
    }

    void ImGuiLayer::DrawPropertyWindow()
    {
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar;
        ImGui::Begin("Property", &m_ShowPropertyWindow, windowFlags);
        
        if (m_SelectedSceneObject != 0)
        {
            const auto& scene = Application::Get().GetScene();
            auto obj = scene.GetSceneObjectWithUUID(m_SelectedSceneObject);
            
            auto& nameComp = obj.GetComponent<NameComponent>();
            constexpr size_t nameBufferSize = 256;
            char nameBuffer[nameBufferSize] = {};
            strcpy_s(nameBuffer, nameComp.Name.c_str());

            if (ImGui::InputText("Name", nameBuffer, nameBufferSize, ImGuiInputTextFlags_EnterReturnsTrue))
            {
                std::string& originalName = nameComp.Name;
                const std::string newName(nameBuffer);
                originalName = newName;
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            
            ImGui::Text("Transform");
            ImGui::Spacing();
            auto& transComp = obj.GetComponent<TransformComponent>();
            auto& position = transComp.Translation;
            auto& rotation = transComp.Rotation;
            auto& scale = transComp.Scale;
            ImGui::DragFloat3("Position", value_ptr(position), 0.01f);
            ImGui::DragFloat3("Rotation", value_ptr(rotation), 0.01f);
            ImGui::DragFloat3("Scale", value_ptr(scale), 0.01f);

            if (obj.HasComponent<DirectionalLightComponent>())
            {
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();
                ImGui::Text("Directional Light");
                ImGui::Spacing();
                
                auto& dirLightComp = obj.GetComponent<DirectionalLightComponent>();
                ImGui::Checkbox("Cast Shadows", &dirLightComp.CastShadows);
                ImGui::Checkbox("Soft Shadows", &dirLightComp.SoftShadows);
                ImGui::ColorEdit3("Radiance", value_ptr(dirLightComp.Radiance));
                ImGui::DragFloat("Intensity", &dirLightComp.Intensity, 0.1f);
                ImGui::DragFloat("Light Size", &dirLightComp.LightSize, 0.1f); // For PCSS
                ImGui::DragFloat("Shadow Amount", &dirLightComp.ShadowAmount, 0.1f);
            }

            if (obj.HasComponent<ModelComponent>())
            {
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();
                ImGui::Text("Model");
                ImGui::Spacing();

                const auto& modelComp = obj.GetComponent<ModelComponent>();
                auto model = ModelManager::GetInstance().GetModelByID(modelComp.ModelID);
                ModelVisitor visitor;
                model->Accept(visitor);
            }
        }
        
        ImGui::End();
    }

    void ImGuiLayer::DrawToneMappingSettingsWindow()
    {
        ImGuiWindowFlags windowFlags = 0;
        ImGui::Begin("Tone Mapping Settings", &m_ShowToneMappingSettings, windowFlags);
        ImGui::TextWrapped("Use the Exposure slider to adjust the overall exposure of the HDR scene.");
        ImGui::SliderFloat("Exposure", &g_ToneMappingParameters.Exposure, -10.0f, 10.0f);
        ImGui::SameLine();
        ShowHelpMarker("Adjust the overall exposure of the HDR scene.");
        ImGui::SliderFloat("Gamma", &g_ToneMappingParameters.Gamma, 0.01f, 5.0f);
        ImGui::SameLine();
        ShowHelpMarker("Adjust the Gamma of the output image.");

        const char* toneMappingMethods[] = {"Linear", "Reinhard", "Reinhard Squared", "ACES Filmic"};

        ImGui::Combo("Tonemapping Methods", reinterpret_cast<int*>(&g_ToneMappingParameters.ToneMappingMethod),
                     toneMappingMethods, 4);

        switch (g_ToneMappingParameters.ToneMappingMethod)
        {
        case TM_Linear:
            ImGui::PlotLines("Linear Tonemapping", &LinearTonemappingPlot, nullptr, VALUES_COUNT, 0, nullptr, 0.0f,
                             1.0f, ImVec2(0, 250));
            ImGui::SliderFloat("Max Brightness", &g_ToneMappingParameters.MaxLuminance, 1.0f, HDR_MAX);
            ImGui::SameLine();
            ShowHelpMarker("Linearly scale the HDR image by the maximum brightness.");
            break;
        case TM_Reinhard:
            ImGui::PlotLines("Reinhard Tonemapping", &ReinhardTonemappingPlot, nullptr, VALUES_COUNT, 0, nullptr,
                             0.0f, 1.0f, ImVec2(0, 250));
            ImGui::SliderFloat("Reinhard Constant", &g_ToneMappingParameters.K, 0.01f, 10.0f);
            ImGui::SameLine();
            ShowHelpMarker("The Reinhard constant is used in the denominator.");
            break;
        case TM_ReinhardSq:
            ImGui::PlotLines("Reinhard Squared Tonemapping", &ReinhardSqrTonemappingPlot, nullptr, VALUES_COUNT, 0,
                             nullptr, 0.0f, 1.0f, ImVec2(0, 250));
            ImGui::SliderFloat("Reinhard Constant", &g_ToneMappingParameters.K, 0.01f, 10.0f);
            ImGui::SameLine();
            ShowHelpMarker("The Reinhard constant is used in the denominator.");
            break;
        case TM_ACESFilmic:
            ImGui::PlotLines("ACES Filmic Tonemapping", &ACESFilmicTonemappingPlot, nullptr, VALUES_COUNT, 0,
                             nullptr, 0.0f, 1.0f, ImVec2(0, 250));
            ImGui::SliderFloat("Shoulder Strength", &g_ToneMappingParameters.A, 0.01f, 5.0f);
            ImGui::SliderFloat("Linear Strength", &g_ToneMappingParameters.B, 0.0f, 100.0f);
            ImGui::SliderFloat("Linear Angle", &g_ToneMappingParameters.C, 0.0f, 1.0f);
            ImGui::SliderFloat("Toe Strength", &g_ToneMappingParameters.D, 0.01f, 1.0f);
            ImGui::SliderFloat("Toe Numerator", &g_ToneMappingParameters.E, 0.0f, 10.0f);
            ImGui::SliderFloat("Toe Denominator", &g_ToneMappingParameters.F, 1.0f, 10.0f);
            ImGui::SliderFloat("Linear White", &g_ToneMappingParameters.LinearWhite, 1.0f, 120.0f);
            break;
        default:
            break;
        }

        if (ImGui::Button("Reset to Defaults"))
        {
            ToneMappingMethod method = g_ToneMappingParameters.ToneMappingMethod;
            g_ToneMappingParameters = ToneMappingParameters();
            g_ToneMappingParameters.ToneMappingMethod = method;
        }
        ImGui::End();
    }

    void ImGuiLayer::DrawHierarchyNode(SceneObject& obj)
    {
        ImGuiTreeNodeFlags flags =
            ImGuiTreeNodeFlags_OpenOnArrow |
            ImGuiTreeNodeFlags_OpenOnDoubleClick |
            ImGuiTreeNodeFlags_SpanFullWidth;
        
        const auto name = obj.GetComponent<NameComponent>();
        const auto id = obj.GetComponent<IDComponent>().ID;

        if (id == m_SelectedSceneObject)
        {
            flags |= ImGuiTreeNodeFlags_Selected;
        }

        if (obj.Children().empty())
        {
            flags |= ImGuiTreeNodeFlags_Leaf;
        }
        
        const auto isOpen = ImGui::TreeNodeEx(name.Name.c_str(), flags);
        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
        {
            m_SelectedSceneObject = id;
        }
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoDisableHover))
        {
            ImGui::SetDragDropPayload("SceneObjectID", &id, sizeof UUID);
            ImGui::Text(name.Name.c_str());
            ImGui::EndDragDropSource();
        }
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SceneObjectID"))
            {
                const auto& scene = Application::Get().GetScene();
                const UUID sourceID = *static_cast<const UUID*>(payload->Data);
                if (sourceID != id)
                {
                    auto source = scene.GetSceneObjectWithUUID(sourceID);

                    source.SetParent(obj);
                }
            }
            ImGui::EndDragDropTarget();
        }

        if (isOpen)
        {
            if (obj.Children().empty())
            {
                ImGui::TreePop();
            }
            else
            {
                const auto& scene = Application::Get().GetScene();
                const auto children = obj.Children();
                for (const auto child : children)
                {
                    auto childObj = scene.GetSceneObjectWithUUID(child);
                    DrawHierarchyNode(childObj);
                }
                ImGui::TreePop();
            }
        }
    }

    void ImGuiLayer::DrawGizmo()
    {
        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetDrawlist();
        ImGuizmo::SetRect(m_SceneWindowPosX, m_SceneWindowPosY, m_SceneWindowWidth, m_SceneWindowHeight);

        bool snap = Input::IsKeyPressed(KeyCode::LeftControl);
        float snapVal = 0;
        switch (m_GizmoType)
        {
        case ImGuizmo::OPERATION::TRANSLATE: snapVal = 0.5f;
        case ImGuizmo::OPERATION::ROTATE: snapVal = 45.0f;
        case ImGuizmo::OPERATION::SCALE: snapVal = 0.5f;
        }
        float snapValues[3] = { snapVal, snapVal, snapVal };
        
        if (m_SelectedSceneObject != 0)
        {
            auto& scene = Application::Get().GetScene();

            auto cam = scene.GetCamera();
            auto& proj = cam->GetProjectionMatrix();
            auto& view = cam->GetViewMatrix();
            
            const auto& obj = scene.GetSceneObjectWithUUID(m_SelectedSceneObject);
            auto& entityTransform = obj.Transform();
            auto transform = scene.GetWorldSpaceTransformMatrix(obj);

            Manipulate(value_ptr(view), value_ptr(proj), static_cast<ImGuizmo::OPERATION>(m_GizmoType), ImGuizmo::LOCAL, value_ptr(transform), nullptr, snap ? snapValues : nullptr);

            if (ImGuizmo::IsUsing())
            {
                auto parent = scene.TryGetSceneObjectWithUUID(obj.GetParentUUID());
                if (parent)
                {
                    glm::mat4 parentTransform = scene.GetWorldSpaceTransformMatrix(parent);
                    transform = inverse(parentTransform) * transform;

                    glm::vec3 translation, rotation, scale;
                    Math::DecomposeTransform(transform, translation, rotation, scale);

                    glm::vec3 deltaRotation = rotation - entityTransform.Rotation;
                    entityTransform.Translation = translation;
                    entityTransform.Rotation += deltaRotation;
                    entityTransform.Scale = scale;
                }
                else
                {
                    glm::vec3 translation, rotation, scale;
                    Math::DecomposeTransform(transform, translation, rotation, scale);

                    glm::vec3 deltaRotation = rotation - entityTransform.Rotation;
                    entityTransform.Translation = translation;
                    entityTransform.Rotation += deltaRotation;
                    entityTransform.Scale = scale;
                }
            }
        }
    }

    void ImGuiLayer::SetStyle()
    {
        ImGuiStyle& style = ImGui::GetStyle();
        style.FrameRounding = 3;
        style.GrabRounding = 3;
        ImVec4* colors = style.Colors;
        colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
        colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
        colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
        colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_FrameBg] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.17f, 0.17f, 0.17f, 1.00f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
        colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.09f, 0.09f, 0.09f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
        colors[ImGuiCol_MenuBarBg] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
        colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
        colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        colors[ImGuiCol_Button] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.17f, 0.17f, 0.17f, 1.00f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.36f, 0.36f, 0.36f, 1.00f);
        colors[ImGuiCol_Header] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.17f, 0.17f, 0.17f, 1.00f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.36f, 0.36f, 0.36f, 1.00f);
        colors[ImGuiCol_Separator] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
        colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
        colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
        colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.20f);
        colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
        colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
        colors[ImGuiCol_Tab] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
        colors[ImGuiCol_TabHovered] = ImVec4(0.17f, 0.17f, 0.17f, 1.00f);
        colors[ImGuiCol_TabActive] = ImVec4(0.17f, 0.17f, 0.17f, 1.00f);
        colors[ImGuiCol_TabUnfocused] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.17f, 0.17f, 0.17f, 1.00f);
        colors[ImGuiCol_DockingPreview] = ImVec4(0.29f, 0.29f, 0.29f, 0.91f);
        colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
        colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
        colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
        colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
        colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
        colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
        colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
        colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
        colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
        colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
        colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
        colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
        colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
        colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
    }

    bool ImGuiLayer::OnKeyPressedEvent(KeyPressedEvent& e)
    {
        if (m_IsSceneWindowHovered && !Input::IsMouseButtonPressed(MouseButton::Right))
        {
            switch (e.GetKeyCode())
            {
            case KeyCode::Q:
                m_GizmoType = -1;
                break;
            case KeyCode::W:
                m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
                break;
            case KeyCode::E:
                m_GizmoType = ImGuizmo::OPERATION::ROTATE;
                break;
            case KeyCode::R:
                m_GizmoType = ImGuizmo::OPERATION::SCALE;
                break;
            case KeyCode::F:
                {
                    if (m_SelectedSceneObject != 0)
                    {
                        auto& scene = Application::Get().GetScene();
                        const auto& cam = scene.GetCamera();

                        const auto obj = scene.GetSceneObjectWithUUID(m_SelectedSceneObject);
                        cam->Focus(obj.Transform().Translation);
                    }
                    break;
                }
            }
        }

        switch (e.GetKeyCode())
        {
        case KeyCode::Delete:
            {
                if (m_SelectedSceneObject != 0)
                {
                    auto& scene = Application::Get().GetScene();
                    const auto obj = scene.GetSceneObjectWithUUID(m_SelectedSceneObject);
                    scene.DestroySceneObject(obj);
                    m_SelectedSceneObject = 0;
                }
                break;
            }
        }

        return false;
    }

    // Helper to display a little (?) mark which shows a tooltip when hovered.
    void ImGuiLayer::ShowHelpMarker( const char* desc )
    {
        ImGui::TextDisabled( "(?)" );
        if ( ImGui::IsItemHovered() )
        {
            ImGui::BeginTooltip();
            ImGui::PushTextWrapPos( ImGui::GetFontSize() * 35.0f );
            ImGui::TextUnformatted( desc );
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
    }

    float ImGuiLayer::LinearTonemapping( float HDR, float max )
    {
        if ( max > 0.0f )
        {
            return std::clamp( HDR / max, 0.0f, 1.0f );
        }
        return HDR;
    }

    float ImGuiLayer::LinearTonemappingPlot( void*, int index )
    {
        return LinearTonemapping( index / (float)VALUES_COUNT * HDR_MAX, g_ToneMappingParameters.MaxLuminance );
    }

    // Reinhard tone mapping.
    // See: http://www.cs.utah.edu/~reinhard/cdrom/tonemap.pdf
    float ImGuiLayer::ReinhardTonemapping( float HDR, float k )
    {
        return HDR / ( HDR + k );
    }

    float ImGuiLayer::ReinhardTonemappingPlot( void*, int index )
    {
        return ReinhardTonemapping( index / (float)VALUES_COUNT * HDR_MAX, g_ToneMappingParameters.K );
    }

    float ImGuiLayer::ReinhardSqrTonemappingPlot( void*, int index )
    {
        float reinhard = ReinhardTonemapping( index / (float)VALUES_COUNT * HDR_MAX, g_ToneMappingParameters.K );
        return reinhard * reinhard;
    }

    // ACES Filmic
    // See: https://www.slideshare.net/ozlael/hable-john-uncharted2-hdr-lighting/142
    float ImGuiLayer::ACESFilmicTonemapping( float x, float A, float B, float C, float D, float E, float F )
    {
        return ( x * ( A * x + C * B ) + D * E ) / ( x * ( A * x + B ) + D * F ) - E / F;
    }

    float ImGuiLayer::ACESFilmicTonemappingPlot( void*, int index )
    {
        float HDR = index / (float)VALUES_COUNT * HDR_MAX;
        return ACESFilmicTonemapping( HDR, g_ToneMappingParameters.A, g_ToneMappingParameters.B, g_ToneMappingParameters.C,
                                      g_ToneMappingParameters.D, g_ToneMappingParameters.E, g_ToneMappingParameters.F ) /
               ACESFilmicTonemapping( g_ToneMappingParameters.LinearWhite, g_ToneMappingParameters.A, g_ToneMappingParameters.B,
                                      g_ToneMappingParameters.C, g_ToneMappingParameters.D, g_ToneMappingParameters.E,
                                      g_ToneMappingParameters.F );
    }

    void ModelVisitor::Visit(Scene& scene)
    {
    }

    void ModelVisitor::Visit(SceneObject& model)
    {
    }

    void ModelVisitor::Visit(ModelNode& modelNode)
    {
    }

    void ModelVisitor::Visit(Mesh& mesh)
    {
        auto mat = mesh.GetMaterial();
        auto& props = mat->GetMaterialProperties();
        ImGui::Text("Material");
        ImGui::ColorEdit4("Base Color", value_ptr(props.BaseColor));
        ImGui::ColorEdit4("Emissive", value_ptr(props.Emissive));
        ImGui::DragFloat("Opacity", &props.Opacity);
        ImGui::DragFloat("Roughness", &props.Roughness);
        ImGui::DragFloat("Metallic", &props.Metallic);
        ImGui::DragFloat("NormalScale", &props.NormalScale);
        ImGui::Spacing();
    }
}

//--------------------------------------------------------------------------------------
// Get surface information for a particular format
//--------------------------------------------------------------------------------------
void GetSurfaceInfo(_In_ size_t width, _In_ size_t height, _In_ DXGI_FORMAT fmt, size_t* outNumBytes,
                    _Out_opt_ size_t* outRowBytes, _Out_opt_ size_t* outNumRows)
{
    size_t numBytes;
    size_t rowBytes;
    size_t numRows;

    bool bc = false;
    bool packed = false;
    bool planar = false;
    size_t bpe = 0;
    switch (fmt)
    {
    case DXGI_FORMAT_BC1_TYPELESS:
    case DXGI_FORMAT_BC1_UNORM:
    case DXGI_FORMAT_BC1_UNORM_SRGB:
    case DXGI_FORMAT_BC4_TYPELESS:
    case DXGI_FORMAT_BC4_UNORM:
    case DXGI_FORMAT_BC4_SNORM:
        bc = true;
        bpe = 8;
        break;

    case DXGI_FORMAT_BC2_TYPELESS:
    case DXGI_FORMAT_BC2_UNORM:
    case DXGI_FORMAT_BC2_UNORM_SRGB:
    case DXGI_FORMAT_BC3_TYPELESS:
    case DXGI_FORMAT_BC3_UNORM:
    case DXGI_FORMAT_BC3_UNORM_SRGB:
    case DXGI_FORMAT_BC5_TYPELESS:
    case DXGI_FORMAT_BC5_UNORM:
    case DXGI_FORMAT_BC5_SNORM:
    case DXGI_FORMAT_BC6H_TYPELESS:
    case DXGI_FORMAT_BC6H_UF16:
    case DXGI_FORMAT_BC6H_SF16:
    case DXGI_FORMAT_BC7_TYPELESS:
    case DXGI_FORMAT_BC7_UNORM:
    case DXGI_FORMAT_BC7_UNORM_SRGB:
        bc = true;
        bpe = 16;
        break;

    case DXGI_FORMAT_R8G8_B8G8_UNORM:
    case DXGI_FORMAT_G8R8_G8B8_UNORM:
    case DXGI_FORMAT_YUY2:
        packed = true;
        bpe = 4;
        break;

    case DXGI_FORMAT_Y210:
    case DXGI_FORMAT_Y216:
        packed = true;
        bpe = 8;
        break;

    case DXGI_FORMAT_NV12:
    case DXGI_FORMAT_420_OPAQUE:
        planar = true;
        bpe = 2;
        break;

    case DXGI_FORMAT_P010:
    case DXGI_FORMAT_P016:
        planar = true;
        bpe = 4;
        break;
    }

    if (bc)
    {
        size_t numBlocksWide = 0;
        if (width > 0)
        {
            numBlocksWide = std::max<size_t>(1, (width + 3) / 4);
        }
        size_t numBlocksHigh = 0;
        if (height > 0)
        {
            numBlocksHigh = std::max<size_t>(1, (height + 3) / 4);
        }
        rowBytes = numBlocksWide * bpe;
        numRows = numBlocksHigh;
        numBytes = rowBytes * numBlocksHigh;
    }
    else if (packed)
    {
        rowBytes = ((width + 1) >> 1) * bpe;
        numRows = height;
        numBytes = rowBytes * height;
    }
    else if (fmt == DXGI_FORMAT_NV11)
    {
        rowBytes = ((width + 3) >> 2) * 4;
        numRows = height * 2; // Direct3D makes this simplifying assumption, although it is larger than the 4:1:1 data
        numBytes = rowBytes * numRows;
    }
    else if (planar)
    {
        rowBytes = ((width + 1) >> 1) * bpe;
        numBytes = rowBytes * height + ((rowBytes * height + 1) >> 1);
        numRows = height + ((height + 1) >> 1);
    }
    else
    {
        size_t bpp = DirectX::BitsPerPixel(fmt);
        rowBytes = (width * bpp + 7) / 8; // round up to nearest byte
        numRows = height;
        numBytes = rowBytes * height;
    }

    if (outNumBytes)
    {
        *outNumBytes = numBytes;
    }
    if (outRowBytes)
    {
        *outRowBytes = rowBytes;
    }
    if (outNumRows)
    {
        *outNumRows = numRows;
    }
}
