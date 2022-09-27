#pragma once
#include <map>
#include <entt/entt.hpp>
#include "UUID.h"

namespace Akari
{
    class Model;
    class Material;
    class SceneObject;
    class PerspectiveCamera;
    struct TransformComponent;
    
    class Scene
    {
    public:
        Scene(const std::string& name = "Untitled Scene");
        ~Scene();

        SceneObject CreateSceneObject(const std::string& name = "Scene Object");
        SceneObject CreateChildSceneObject(SceneObject parent, const std::string& name = "Scene Object");
        SceneObject CreateSceneObjectWithID(UUID uuid, const std::string& name = "Scene Object");
        SceneObject DuplicateSceneObject(SceneObject object);
        void DestroySceneObject(SceneObject object, bool excludeChildren = false, bool first = true);

        template<typename... Components>
        auto GetAllSceneObjectsWith()
        {
            return m_Registry.view<Components...>();
        }

        // return SceneObject with id as specified. entity is expected to exist (runtime error if it doesn't)
        SceneObject GetSceneObjectWithUUID(UUID id) const;

        // return entity with id as specified, or empty entity if cannot be found - caller must check
        SceneObject TryGetSceneObjectWithUUID(UUID id) const;

        // return entity with tag as specified, or empty entity if cannot be found - caller must check
        SceneObject TryGetSceneObjectWithName(const std::string& name);

        void ConvertToLocalSpace(SceneObject sceneObject);
        void ConvertToWorldSpace(SceneObject sceneObject);
        Math::AffineTransform GetWorldSpaceTransformMatrix(SceneObject sceneObject);
        TransformComponent GetWorldSpaceTransform(SceneObject sceneObject);

        void ParentSceneObject(SceneObject sceneObject, SceneObject parent);
        void UnparentSceneObject(SceneObject sceneObject, bool convertToWorldSpace = true);

        UUID GetUUID() const { return m_SceneID; }
        std::shared_ptr<PerspectiveCamera> GetCamera();
        
    private:
        void SortSceneObjects();
        
        UUID m_SceneID;
        entt::entity m_SceneEntity = entt::null;
        entt::registry m_Registry;

        std::map<UUID, std::shared_ptr<Model>> m_ModelDic;
        std::map<UUID, std::shared_ptr<Material>> m_MaterialDic;
        std::unordered_map<UUID, SceneObject> m_SceneObjectIDMap;

        std::shared_ptr<PerspectiveCamera> m_Camera;

        friend class SceneObject;
    };
}
