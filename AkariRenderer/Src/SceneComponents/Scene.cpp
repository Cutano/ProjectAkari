#include "pch.h"
#include "Scene.h"
#include "SceneObject.h"

namespace Akari
{
    Scene::Scene(const std::string& name)
    {
    }

    Scene::~Scene()
    {
    }

    SceneObject Scene::CreateSceneObject(const std::string& name)
    {
        return SceneObject({}, *this);
    }

    SceneObject Scene::CreateChildSceneObject(SceneObject parent, const std::string& name)
    {
        return SceneObject({}, *this);
    }

    SceneObject Scene::CreateSceneObjectWithID(UUID uuid, const std::string& name)
    {
        return SceneObject({}, *this);
    }

    SceneObject Scene::DuplicateSceneObject(SceneObject object)
    {
        return SceneObject({}, *this);
    }

    void Scene::DestroySceneObject(SceneObject object)
    {
    }
}
