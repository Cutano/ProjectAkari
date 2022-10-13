#pragma once
#include "Components.h"

namespace Akari
{
    struct alignas(16) DirectionalLight
    {
        TransformComponent Trans;
        DirectionalLightComponent Props;
    };

    struct alignas(16) PointLight
    {
        TransformComponent Trans;
        PointLightComponent Props;
    };
}
