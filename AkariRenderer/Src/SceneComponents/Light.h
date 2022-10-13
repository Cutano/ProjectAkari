#pragma once
#include "Components.h"

namespace Akari
{
    struct DirectionalLight
    {
        TransformComponent Trans;
        DirectionalLightComponent Props;
    };

    struct PointLight
    {
        TransformComponent Trans;
        PointLightComponent Props;
    };
}
