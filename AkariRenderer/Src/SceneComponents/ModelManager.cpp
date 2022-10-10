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
            const auto cube = cmd->CreateCube();
            m_CubeID = cubeID;
            m_ModelRegistry[cubeID] = *cube;
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
        m_ModelRegistry[id] = *model;
        return id;
    }

    Model& ModelManager::GetModelByID(UUID id)
    {
        return m_ModelRegistry[id];
    }

    UUID ModelManager::GetCubeID()
    {
        return m_CubeID;
    }
}
