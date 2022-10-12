#pragma once
#include <unordered_map>

#include "UUID.h"

namespace Akari
{
    class Model;
    
    class ModelManager
    {
    public:
        static ModelManager& GetInstance()
        {
            static ModelManager instance;
            return instance;
        }

        ~ModelManager() = default;
        ModelManager(ModelManager const&) = delete;
        ModelManager(ModelManager const&&) = delete;
        void operator=(ModelManager const&) = delete;
        void operator=(ModelManager const&&) = delete;
        
        void Init();
        void Shutdown();

        UUID LoadModelFromFile(std::wstring& path);

        std::shared_ptr<Model> GetModelByID(UUID id);

        UUID GetCubeID();

    private:
        ModelManager() = default;
        
        std::unordered_map<UUID, std::shared_ptr<Model>> m_ModelRegistry{};

        // Default Geometries
        UUID m_CubeID{0};
    };
    
}
