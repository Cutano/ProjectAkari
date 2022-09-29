#pragma once

namespace Akari::Math
{
    bool DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale);

    //TODO: Replace with a C++20 concept?
    template<typename T, std::enable_if_t<std::is_integral_v<T>, bool> = true>
    static T DivideAndRoundUp(T dividend, T divisor)
    {
        return (dividend + divisor - 1) / divisor;
    }

    // Only integers are allowed
    template<typename T, typename DivisorT, std::enable_if_t<std::is_same_v<T, glm::uvec2> && std::is_integral_v<DivisorT>, bool> = true>
    static T DivideAndRoundUp(T dividend, DivisorT divisor)
    {
        return { DivideAndRoundUp(dividend.x, divisor), DivideAndRoundUp(dividend.y, divisor) };
    }
    
    /***************************************************************************
    * These functions were taken from the MiniEngine.
    * Source code available here:
    * https://github.com/Microsoft/DirectX-Graphics-Samples/blob/master/MiniEngine/Core/Math/Common.h
    * Retrieved: January 13, 2016
    **************************************************************************/
    template <typename T>
    inline T AlignUpWithMask(T value, size_t mask)
    {
        return (T)(((size_t)value + mask) & ~mask);
    }

    template <typename T>
    inline T AlignDownWithMask(T value, size_t mask)
    {
        return (T)((size_t)value & ~mask);
    }

    template <typename T>
    inline T AlignUp(T value, size_t alignment)
    {
        return AlignUpWithMask(value, alignment - 1);
    }

    template <typename T>
    inline T AlignDown(T value, size_t alignment)
    {
        return AlignDownWithMask(value, alignment - 1);
    }

    template <typename T>
    inline bool IsAligned(T value, size_t alignment)
    {
        return 0 == ((size_t)value & (alignment - 1));
    }

    template <typename T>
    inline T DivideByMultiple(T value, size_t alignment)
    {
        return (T)((value + alignment - 1) / alignment);
    }
    /***************************************************************************/
}
