#include "pch.h"
#include "Scene.h"
#include "SceneObject.h"
#include "RHI/Renderer.h"
#include "Camera/PerspectiveCamera.h"
#include "Components.h"
#include "RHI/RenderTarget.h"

namespace Akari
{
    Scene::Scene(const std::string& name)
    {
        m_Camera = std::make_shared<PerspectiveCamera>();
        auto camGO = CreateSceneObject("Camera");
        auto& camComponent = camGO.AddComponent<CameraComponent>();
        auto& camTrans = camGO.GetComponent<TransformComponent>();
        camTrans.Translation = Math::Vector3(0.0f, 1.0f, 0.0f);
        m_Camera->SetPosition(camTrans.Translation);
        m_Camera->SetLookDirection({0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f});

        if (camComponent.IsPerspective)
        {
            const auto& rt = Renderer::GetInstance().GetMsaaRenderTarget();
            camComponent.AspectRatio = static_cast<float>(rt->GetHeight()) / rt->GetWidth();
            m_Camera->SetFOV(camComponent.VerticalFOV);
            m_Camera->SetAspectRatio(camComponent.AspectRatio);
            m_Camera->SetZRange(camComponent.NearClip, camComponent.FarClip);
        }
    }

    Scene::~Scene()
    {
    }
    
    template<typename T>
    static void CopyComponent(entt::registry& dstRegistry, entt::registry& srcRegistry, const std::unordered_map<UUID, entt::entity>& enttMap)
    {
        auto components = srcRegistry.view<T>();
        for (auto srcEntity : components)
        {
            entt::entity destEntity = enttMap.at(srcRegistry.get<IDComponent>(srcEntity).ID);

            auto& srcComponent = srcRegistry.get<T>(srcEntity);
            auto& destComponent = dstRegistry.emplace_or_replace<T>(destEntity, srcComponent);
        }
    }

    template<typename T>
    static void CopyComponentIfExists(entt::entity dst, entt::entity src, entt::registry& registry)
    {
        if (registry.all_of<T>(src))
        {
            auto& srcComponent = registry.get<T>(src);
            registry.emplace_or_replace<T>(dst, srcComponent);
        }
    }

    template<typename T>
    static void CopyComponentIfExists(entt::entity dst, entt::registry& dstRegistry, entt::entity src, entt::registry& srcRegistry)
    {
        if (srcRegistry.all_of<T>(src))
        {
            auto& srcComponent = srcRegistry.get<T>(src);
            dstRegistry.emplace_or_replace<T>(dst, srcComponent);
        }
    }

    SceneObject Scene::CreateSceneObject(const std::string& name)
    {
        auto sceneObject = SceneObject { m_Registry.create(), this };
        auto& idComponent = sceneObject.AddComponent<IDComponent>();
        idComponent.ID = {};

        sceneObject.AddComponent<TransformComponent>();
        if (!name.empty())
            sceneObject.AddComponent<NameComponent>(name);

        sceneObject.AddComponent<RelationshipComponent>();

        m_SceneObjectIDMap.try_emplace(idComponent.ID, sceneObject);

        SortSceneObjects();

        return sceneObject;
    }

    SceneObject Scene::CreateChildSceneObject(SceneObject parent, const std::string& name)
    {
        auto sceneObject = CreateSceneObject(name);
        sceneObject.SetParent(parent);

        return sceneObject;
    }

    SceneObject Scene::CreateSceneObjectWithID(UUID uuid, const std::string& name)
    {
        auto entity = SceneObject { m_Registry.create(), this };
        auto& idComponent = entity.AddComponent<IDComponent>();
        idComponent.ID = uuid;

        entity.AddComponent<TransformComponent>();
        if (!name.empty())
            entity.AddComponent<NameComponent>(name);

        entity.AddComponent<RelationshipComponent>();

        assert(!m_SceneObjectIDMap.contains(uuid));
        m_SceneObjectIDMap[uuid] = entity;

        SortSceneObjects();

        return entity;
    }

    SceneObject Scene::DuplicateSceneObject(SceneObject object)
    {
        auto parentNewEntity = [&object, scene = this](SceneObject newEntity)
        {
            if (auto parent = object.GetParent(); parent)
            {
                newEntity.SetParentUUID(parent.GetUUID());
                parent.Children().push_back(newEntity.GetUUID());
            }
        };

        SceneObject newObject;
        if (object.HasComponent<NameComponent>())
            newObject = CreateSceneObject(object.GetComponent<NameComponent>().Name);
        else
            newObject = CreateSceneObject();

        CopyComponentIfExists<TransformComponent>(newObject.m_EntityHandle, object.m_EntityHandle, m_Registry);
        CopyComponentIfExists<DirectionalLightComponent>(newObject.m_EntityHandle, object.m_EntityHandle, m_Registry);
        CopyComponentIfExists<PointLightComponent>(newObject.m_EntityHandle, object.m_EntityHandle, m_Registry);
        CopyComponentIfExists<ModelComponent>(newObject.m_EntityHandle, object.m_EntityHandle, m_Registry);

        auto childIds = object.Children(); // need to take a copy of children here, because the collection is mutated below
        for (auto childId : childIds)
        {
            SceneObject childDuplicate = DuplicateSceneObject(GetSceneObjectWithUUID(childId));

            // At this point childDuplicate is a child of entity, we need to remove it from that entity
            UnparentSceneObject(childDuplicate, false);

            childDuplicate.SetParentUUID(newObject.GetUUID());
            newObject.Children().push_back(childDuplicate.GetUUID());
        }

        parentNewEntity(newObject);

        return newObject;
    }

    void Scene::DestroySceneObject(SceneObject object, bool excludeChildren, bool first)
    {
        if (!excludeChildren)
        {
            for (size_t i = 0; i < object.Children().size(); i++)
            {
                auto childId = object.Children()[i];
                SceneObject child = GetSceneObjectWithUUID(childId);
                DestroySceneObject(child, excludeChildren, false);
            }
        }

        if (first)
        {
            if (auto parent = object.GetParent(); parent)
                parent.RemoveChild(object);
        }

        m_SceneObjectIDMap.erase(object.GetUUID());
        m_Registry.destroy(object.m_EntityHandle);

        SortSceneObjects();
    }

    SceneObject Scene::GetSceneObjectWithUUID(UUID id) const
    {
        assert(m_SceneObjectIDMap.contains(id) && "Invalid entity ID or entity doesn't exist in scene!");
        return m_SceneObjectIDMap.at(id);
    }

    SceneObject Scene::TryGetSceneObjectWithUUID(UUID id) const
    {
        if (const auto iter = m_SceneObjectIDMap.find(id); iter != m_SceneObjectIDMap.end())
            return iter->second;
        return SceneObject {};
    }

    SceneObject Scene::TryGetSceneObjectWithName(const std::string& name)
    {
        const auto entities = GetAllSceneObjectsWith<NameComponent>();
        for (const auto e : entities)
        {
            if (entities.get<NameComponent>(e).Name == name)
                return SceneObject(e, const_cast<Scene*>(this));
        }

        return SceneObject {};
    }

    void Scene::ConvertToLocalSpace(SceneObject sceneObject)
    {
        const SceneObject parent = TryGetSceneObjectWithUUID(sceneObject.GetParentUUID());
        if (parent.m_Scene == nullptr) return;

        TransformComponent& transComp = sceneObject.Transform();
        const TransformComponent parentTrans = GetWorldSpaceTransform(parent);
        transComp.Translation -= parentTrans.Translation;
        transComp.Rotation -= parentTrans.Rotation;
        transComp.Scale -= parentTrans.Scale;
    }

    void Scene::ConvertToWorldSpace(SceneObject sceneObject)
    {
        const SceneObject parent = TryGetSceneObjectWithUUID(sceneObject.GetParentUUID());
        if (parent.m_Scene == nullptr) return;

        TransformComponent& transComp = sceneObject.Transform();
        const TransformComponent worldSpaceTransform = GetWorldSpaceTransform(sceneObject);
        transComp.Translation = worldSpaceTransform.Translation;
        transComp.Rotation = worldSpaceTransform.Rotation;
        transComp.Scale = worldSpaceTransform.Scale;
    }

    Math::AffineTransform Scene::GetWorldSpaceTransformMatrix(SceneObject sceneObject)
    {
        SceneObject parent = TryGetSceneObjectWithUUID(sceneObject.GetParentUUID());
        if (parent)
            return GetWorldSpaceTransformMatrix(parent) * sceneObject.Transform().GetTransform();

        return sceneObject.Transform().GetTransform();
    }

    TransformComponent Scene::GetWorldSpaceTransform(SceneObject sceneObject)
    {
        TransformComponent transComp = sceneObject.Transform();
        const SceneObject parent = TryGetSceneObjectWithUUID(sceneObject.GetParentUUID());
        if (parent.m_Scene != nullptr)
        {
            const TransformComponent parentTrans = GetWorldSpaceTransform(parent);
            transComp.Translation += parentTrans.Translation;
            transComp.Rotation += parentTrans.Rotation;
            transComp.Scale += parentTrans.Scale;
        }

        return transComp;
    }

    void Scene::ParentSceneObject(SceneObject sceneObject, SceneObject parent)
    {
        if (parent.IsDescendantOf(sceneObject))
        {
            UnparentSceneObject(parent);

            SceneObject newParent = TryGetSceneObjectWithUUID(sceneObject.GetParentUUID());
            if (newParent)
            {
                UnparentSceneObject(sceneObject);
                ParentSceneObject(parent, newParent);
            }
        }
        else
        {
            SceneObject previousParent = TryGetSceneObjectWithUUID(sceneObject.GetParentUUID());

            if (previousParent)
                UnparentSceneObject(sceneObject);
        }

        sceneObject.SetParentUUID(parent.GetUUID());
        parent.Children().push_back(sceneObject.GetUUID());

        ConvertToLocalSpace(sceneObject);
    }

    void Scene::UnparentSceneObject(SceneObject sceneObject, bool convertToWorldSpace)
    {
        SceneObject parent = TryGetSceneObjectWithUUID(sceneObject.GetParentUUID());
        if (!parent)
            return;

        auto& parentChildren = parent.Children();
        parentChildren.erase(std::remove(parentChildren.begin(), parentChildren.end(), sceneObject.GetUUID()), parentChildren.end());

        if (convertToWorldSpace)
            ConvertToWorldSpace(sceneObject);

        sceneObject.SetParentUUID(0);
    }

    std::shared_ptr<PerspectiveCamera> Scene::GetCamera()
    {
        return m_Camera;
    }

    void Scene::SortSceneObjects()
    {
        m_Registry.sort<IDComponent>([&](const auto lhs, const auto rhs)
        {
            auto lhsSceneObject = m_SceneObjectIDMap.find(lhs.ID);
            auto rhsSceneObject = m_SceneObjectIDMap.find(rhs.ID);
            return static_cast<uint32_t>(lhsSceneObject->second) < static_cast<uint32_t>(rhsSceneObject->second);
        });
    }
}
