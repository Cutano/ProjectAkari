#pragma once
#include "Scene.h"
#include "Components.h"

namespace Akari
{
    class SceneObject
    {
    public:
        SceneObject() = default;
        SceneObject(entt::entity handle, Scene* scene) : m_EntityHandle(handle), m_Scene(scene) {}

        template<typename T, typename... Args>
        T& AddComponent(Args&&... args)
        {
            assert(!HasComponent<T>() && "Entity already has component!");
            return m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
        }

        template<typename T>
        T& GetComponent()
        {
            assert(HasComponent<T>() && "Entity doesn't have component!");
            return m_Scene->m_Registry.get<T>(m_EntityHandle);
        }

        template<typename T>
        const T& GetComponent() const
        {
            assert(HasComponent<T>() && "Entity doesn't have component!");
            return m_Scene->m_Registry.get<T>(m_EntityHandle);
        }

        template<typename... T>
        bool HasComponent()
        {
        	return m_Scene->m_Registry.all_of<T...>(m_EntityHandle);
        }
        
        template<typename... T>
        bool HasComponent() const
        {
        	return m_Scene->m_Registry.all_of<T...>(m_EntityHandle);
        }
        
        template<typename...T>
        bool HasAny()
        {
        	return m_Scene->m_Registry.any_of<T...>(m_EntityHandle);
        }
        
        template<typename...T>
        bool HasAny() const
        {
        	return m_Scene->m_Registry.any_of<T...>(m_EntityHandle);
        }

        template<typename T>
        void RemoveComponent()
        {
            assert(HasComponent<T>() && "Entity doesn't have component!");
            m_Scene->m_Registry.remove<T>(m_EntityHandle);
        }

        [[nodiscard]] TransformComponent& Transform() const { return m_Scene->m_Registry.get<TransformComponent>(m_EntityHandle); }
        [[nodiscard]] std::string& Name() const { return m_Scene->m_Registry.get<NameComponent>(m_EntityHandle); }

        operator uint32_t () const { return static_cast<uint32_t>(m_EntityHandle); }
        operator entt::entity () const { return m_EntityHandle; }
        operator bool () const { return (m_EntityHandle != entt::null) && m_Scene; }

        bool operator==(const SceneObject& other) const
        {
            return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene;
        }

        bool operator!=(const SceneObject& other) const
        {
            return !(*this == other);
        }

        SceneObject GetParent() const
        {
            return m_Scene->TryGetSceneObjectWithUUID(GetParentUUID());
        }

        void SetParent(SceneObject parent)
        {
            SceneObject currentParent = GetParent();
            if (currentParent == parent)
                return;

            // If changing parent, remove child from existing parent
            if (currentParent)
                currentParent.RemoveChild(*this);

            // Setting to null is okay
            SetParentUUID(parent.GetUUID());

            if (parent)
            {
                auto& parentChildren = parent.Children();
                UUID uuid = GetUUID();
                if (std::ranges::find(parentChildren, uuid) == parentChildren.end())
                    parentChildren.emplace_back(GetUUID());
            }
        }

        void SetParentUUID(UUID parent) { GetComponent<RelationshipComponent>().ParentHandle = parent; }
        UUID GetParentUUID() const { return GetComponent<RelationshipComponent>().ParentHandle; }
        std::vector<UUID>& Children() { return GetComponent<RelationshipComponent>().Children; }
        const std::vector<UUID>& Children() const { return GetComponent<RelationshipComponent>().Children; }

        bool RemoveChild(SceneObject child)
        {
            UUID childId = child.GetUUID();
            std::vector<UUID>& children = Children();
            const auto it = std::ranges::find(children, childId);
            if (it != children.end())
            {
                children.erase(it);
                return true;
            }

            return false;
        }

        bool IsAncestorOf(SceneObject entity) const
        {
            const auto& children = Children();

            if (children.empty())
                return false;

            for (UUID child : children)
            {
                if (child == entity.GetUUID())
                    return true;
            }

            for (UUID child : children)
            {
                if (m_Scene->GetSceneObjectWithUUID(child).IsAncestorOf(entity))
                    return true;
            }

            return false;
        }

        bool IsDescendantOf(SceneObject entity) const { return entity.IsAncestorOf(*this); }

        UUID GetUUID() const { return GetComponent<IDComponent>().ID; }
        UUID GetSceneUUID() const { return m_Scene->GetUUID(); }
        
    private:
        entt::entity m_EntityHandle{ entt::null };
        Scene* m_Scene = nullptr;

        friend class Scene;
    };
}
