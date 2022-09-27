#pragma once
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

    struct TransformComponent
    {
        Math::Vector3 Translation = {0.0f, 0.0f, 0.0f};
        Math::Vector3 Rotation = {0.0f, 0.0f, 0.0f};
        Math::Vector3 Scale = {1.0f, 1.0f, 1.0f};

        TransformComponent() = default;
        TransformComponent(const TransformComponent& other) = default;

        TransformComponent(const Math::Vector3& translation)
            : Translation(translation)
        {
        }

        [[nodiscard]] Math::AffineTransform GetTransform() const
        {
            return Math::AffineTransform {
                Math::Matrix3(Math::Quaternion(Rotation.GetX(), Rotation.GetY(), Rotation.GetZ())) *
                Math::Matrix3::MakeScale(Scale),
                Translation
            };
        }
    };

    struct DirectionalLightComponent
    {
        Math::Vector3 Radiance = {1.0f, 1.0f, 1.0f};
        float Intensity = 1.0f;
        bool CastShadows = true;
        bool SoftShadows = true;
        float LightSize = 0.5f; // For PCSS
        float ShadowAmount = 1.0f;
    };

    struct PointLightComponent
    {
        Math::Vector3 Radiance = {1.0f, 1.0f, 1.0f};
        float Intensity = 1.0f;
        float LightSize = 0.5f; // For PCSS
        float MinRadius = 1.f;
        float Radius = 10.f;
        bool CastsShadows = true;
        bool SoftShadows = true;
        float Falloff = 1.f;
    };

    struct ModelComponent
    {
        UUID ModelID;
    };

    struct CameraComponent
    {
        float Exposure = 1.0f;

        bool IsPerspective = true;
        float VerticalFOV = Math::PI / 4.0f;	// Field of view angle in radians
        float AspectRatio = 9.0 / 16.0;
        float NearClip = 0.1f;
        float FarClip = 100.0f;
    };
}
