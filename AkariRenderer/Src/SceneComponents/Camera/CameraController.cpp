#include "pch.h"
#include "CameraController.h"

using namespace Math;

namespace Akari
{
    void CameraController::ApplyMomentum(float& oldValue, float& newValue, float deltaTime)
    {
        float blendedValue;
        if (Abs(newValue) > Abs(oldValue))
            blendedValue = Lerp(newValue, oldValue, Pow(0.6f, deltaTime * 60.0f));
        else
            blendedValue = Lerp(newValue, oldValue, Pow(0.8f, deltaTime * 60.0f));
        oldValue = blendedValue;
        newValue = blendedValue;
    }
}
