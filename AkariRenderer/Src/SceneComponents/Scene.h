#pragma once
#include <map>
#include <entt/entt.hpp>
#include "UUID.h"

namespace Akari
{
    class Model;
    class Material;
    class SceneObject;
    
    class Scene
    {
    public:
        Scene(const std::string& name = "Untitled Scene");
        ~Scene();

        SceneObject CreateSceneObject(const std::string& name = "Scene Object");
        SceneObject CreateChildSceneObject(SceneObject parent, const std::string& name = "Scene Object");
        SceneObject CreateSceneObjectWithID(UUID uuid, const std::string& name = "Scene Object");
        SceneObject DuplicateSceneObject(SceneObject object);
        void DestroySceneObject(SceneObject object);
        
    private:
        UUID m_SceneID;
        entt::entity m_SceneEntity = entt::null;
        entt::registry m_Registry;

        std::map<UUID, std::shared_ptr<Model>> m_ModelDic;
        std::map<UUID, std::shared_ptr<Material>> m_MaterialDic;

        friend class SceneObject;
    };
}
