#pragma once

#include "GLFW/glfw3.h"
struct MoveState
{
    bool forward, backward, left, right, up, down;
    inline bool IsMoving() const
    {
        return forward || backward || left || right || up || down;
    }
};
extern bool leftMouseButtonPressed;
extern struct MoveState moveState;
extern float xoffset;
extern float yoffset;

void SetupCallback(GLFWwindow *window);