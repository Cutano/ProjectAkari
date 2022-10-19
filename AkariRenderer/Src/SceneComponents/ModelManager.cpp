#include "pch.h"
#include "ModelManager.h"

#include <DirectXCollision.h>

#include "Model.h"
#include "ModelNode.h"
#include "RHI/CommandList.h"
#include "RHI/Renderer.h"

namespace Akari
{
    void ModelManager::Init()
    {
        {
            const auto cmd = Renderer::GetInstance().GetCommandListCopy();

            const UUID cubeID{};
            m_CubeID = cubeID;
            m_ModelRegistry[cubeID] = cmd->CreateCube(1.0f, false);

            const UUID sphereID{};
            m_SphereID = sphereID;
            m_ModelRegistry[sphereID] = cmd->CreateSphere(0.5f, 16, false);

            Renderer::GetInstance().ExecuteCommandList(cmd);
        }
    }

    void ModelManager::Shutdown()
    {
    }

    UUID ModelManager::LoadModelFromFile(const std::wstring& path, const std::function<bool( float )>& loadingProgress)
    {
        // Load a scene, passing an optional function object for receiving loading progress events.
        const auto& cmd = Renderer::GetInstance().GetCommandListCopy();
        auto model = cmd->LoadModelFromFile( path, loadingProgress);

        if (model)
        {
            // Scale the scene so it fits in the camera frustum.
            DirectX::BoundingSphere s;
            DirectX::BoundingSphere::CreateFromBoundingBox( s, model->GetAABB() );
            auto scale = 50.0f / ( s.Radius * 2.0f );
            s.Radius *= scale;

            model->GetRootNode()->SetLocalTransform(DirectX::XMMatrixScaling( scale, scale, scale ) );
        }
        
        Renderer::GetInstance().ExecuteAndFlushCommandList(cmd);
        
        UUID id{};
        if (model != nullptr)
        {
            m_ModelRegistry[id] = model;
        }
        else
        {
            id = 0;
        }
        return id;
    }

    std::shared_ptr<Model> ModelManager::GetModelByID(UUID id)
    {
        return m_ModelRegistry[id];
    }

    UUID ModelManager::GetCubeID()
    {
        return m_CubeID;
    }

    UUID ModelManager::GetSphereID()
    {
        return m_SphereID;
    }
}
