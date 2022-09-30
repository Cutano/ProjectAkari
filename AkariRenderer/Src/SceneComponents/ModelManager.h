#pragma once
#include <unordered_map>

#include "Model.h"
#include "UUID.h"

namespace Akari
{
    class ModelManager
    {
    public:
        static void Init();
        static void Shutdown();

    private:
        static std::unordered_map<UUID, Model> s_ModelRegistry;
    };
    
}
