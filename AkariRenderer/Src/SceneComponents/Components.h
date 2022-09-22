#pragma once
#include "UUID.h"

namespace Akari
{
    enum class LightType
    {
        None = 0, Directional = 1, Point = 2, Spot = 3
    };
    
    struct IDComponent
    {
        UUID ID = 0;
    };

    struct RelationshipComponent
    {
        UUID ParentHandle = 0;
        std::vector<UUID> Children{};

        RelationshipComponent() = default;
        RelationshipComponent(const RelationshipComponent& other) = default;
        RelationshipComponent(UUID parent)
            : ParentHandle(parent) {}
    };

    struct TransformComponent
    {
        Math::Vector3 Translation = { 0.0f, 0.0f, 0.0f };
        Math::Vector3 Rotation = { 0.0f, 0.0f, 0.0f };
        Math::Vector3 Scale = { 1.0f, 1.0f, 1.0f };

        TransformComponent() = default;
        TransformComponent(const TransformComponent& other) = default;
        TransformComponent(const Math::Vector3& translation)
            : Translation(translation) {}

        [[nodiscard]] Math::AffineTransform GetTransform() const
        {
            Math::AffineTransform trans;
            trans.MakeScale(Scale);
            trans.MakeXRotation(Rotation.GetX());
            trans.MakeYRotation(Rotation.GetY());
            trans.MakeZRotation(Rotation.GetZ());
            trans.MakeTranslation(Translation);
            return trans;
        }
    };

    struct DirectionalLightComponent
    {
        Math::Vector3 Radiance = { 1.0f, 1.0f, 1.0f };
        float Intensity = 1.0f;
        bool CastShadows = true;
        bool SoftShadows = true;
        float LightSize = 0.5f; // For PCSS
        float ShadowAmount = 1.0f;
    };

    struct PointLightComponent
    {
        Math::Vector3 Radiance = { 1.0f, 1.0f, 1.0f };
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

}