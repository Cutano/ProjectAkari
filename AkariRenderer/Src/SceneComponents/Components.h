#pragma once
#include "Math/Math.h"

#include "UUID.h"

namespace Akari
{
    enum class LightType
    {
        None = 0,
        Directional = 1,
        Point = 2,
        Spot = 3
    };

    struct IDComponent
    {
        UUID ID = 0;
    };

    struct NameComponent
    {
        std::string Name;

        NameComponent() = default;
        NameComponent(const NameComponent& other) = default;

        NameComponent(const std::string& name)
            : Name(name)
        {
        }

        operator std::string&() { return Name; }
        operator const std::string&() const { return Name; }
    };

    struct RelationshipComponent
    {
        UUID ParentHandle = 0;
        std::vector<UUID> Children{};

        RelationshipComponent() = default;
        RelationshipComponent(const RelationshipComponent& other) = default;

        RelationshipComponent(UUID parent)
            : ParentHandle(parent)
        {
        }
    };

    struct alignas(16) TransformComponent
    {
        glm::vec3 Translation = { 0.0f, 0.0f, 0.0f };
        glm::vec3 Rotation = { 0.0f, 0.0f, 0.0f };
        glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };
        // ----36byte
        char padding[12] = {0};
        // ----48byte

        TransformComponent() = default;
        TransformComponent(const TransformComponent& other) = default;
        TransformComponent(const glm::vec3& translation)
            : Translation(translation) {}

        [[nodiscard]] glm::mat4 GetTransform() const
        {
            return glm::translate(glm::mat4(1.0f), Translation)
                * glm::toMat4(glm::quat(Rotation))
                * glm::scale(glm::mat4(1.0f), Scale);
        }

        void SetTransform(const glm::mat4& transform)
        {
            Math::DecomposeTransform(transform, Translation, Rotation, Scale);
        }
    };

    struct alignas(16) DirectionalLightComponent
    {
        glm::vec3 Radiance = {1.0f, 1.0f, 1.0f};
        // ----12byte
        float Intensity = 1.0f;
        float LightSize = 0.5f; // For PCSS
        float ShadowAmount = 1.0f;
        // ----24byte
        int CastShadows = true;
        int SoftShadows = true;
        // ----32byte
    };

    struct alignas(16) PointLightComponent
    {
        glm::vec3 Radiance = {1.0f, 1.0f, 1.0f};
        // ----12byte
        float Intensity = 1.0f;
        float LightSize = 0.5f; // For PCSS
        float MinRadius = 1.f;
        float Radius = 10.f;
        float Falloff = 1.f;
        // ----32byte
        int CastsShadows = true;
        int SoftShadows = true;
        // ----40byte
        char padding[8] = {0};
        // ----48byte
    };

    struct ModelComponent
    {
        UUID ModelID;
    };
}
