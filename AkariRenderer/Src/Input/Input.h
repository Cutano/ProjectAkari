#pragma once

#include "KeyCodes.h"
#include <map>

namespace Akari {

    struct Controller
    {
        int ID;
        std::string Name;
        std::map<int, bool> ButtonStates;
        std::map<int, float> AxisStates;
        std::map<int, uint8_t> HatStates;
    };

    class Input
    {
    public:
        static void Update();

        static bool IsKeyPressed(KeyCode keycode);

        static bool IsMouseButtonPressed(MouseButton button);
        static float GetMouseX();
        static float GetMouseY();
        static std::pair<float, float> GetMousePosition();

        static void SetCursorMode(CursorMode mode);
        static CursorMode GetCursorMode();

        // Controllers
        static bool IsControllerPresent(int id);
        static std::vector<int> GetConnectedControllerIDs();
        static const Controller* GetController(int id);
        static std::string_view GetControllerName(int id);
        static bool IsControllerButtonPressed(int controllerID, int button);
        static float GetControllerAxis(int controllerID, int axis);
        static uint8_t GetControllerHat(int controllerID, int hat);
		
        static const std::map<int, Controller>& GetControllers() { return s_Controllers; }
    private:
        inline static std::map<int, Controller> s_Controllers;
        inline static float preX = 0, preY = 0;
    };

}
