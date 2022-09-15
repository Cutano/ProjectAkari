#include "pch.h"
#include <iostream>

#include <glfw/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <glfw/glfw3native.h>

int main()
{
    const int width = 800, height = 600;

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    auto window = glfwCreateWindow(width, height, "glfw - DX12", nullptr, nullptr);
    if (!window)
    {
        std::cout << "Creat window error" << std::endl;
        return -1;
    }

    auto process_keystrokes_input = [](GLFWwindow* window)
    {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, 1);
    };

    auto hMainWnd = glfwGetWin32Window(window);

    while (!glfwWindowShouldClose(window))
    {
        process_keystrokes_input(window);

        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}
