#include "pch.h"
#include "ModelManager.h"

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
            m_ModelRegistry[sphereID] = cmd->CreateSphere(0.5f, 32, false);

            Renderer::GetInstance().ExecuteCommandList(cmd);
        }
    }

    void ModelManager::Shutdown()
    {
    }

    UUID ModelManager::LoadModelFromFile(std::wstring& path)
    {
        const auto model = Renderer::GetInstance().LoadModel(path);
        if (model == nullptr)
        {
            return 0;
        }
        
        UUID id{};
        m_ModelRegistry[id] = model;
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
