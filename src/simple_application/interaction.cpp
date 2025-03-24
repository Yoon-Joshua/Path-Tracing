#include "interaction.h"
#include "application.h"
#include <cstring>

bool leftMouseButtonPressed = false;
struct MoveState moveState = {};

static float lastX, lastY;
static bool firstMouse = true;
float xoffset, yoffset;

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        if (key == GLFW_KEY_W && leftMouseButtonPressed)
        {
            moveState.forward = true;
        }
        if (key == GLFW_KEY_S && leftMouseButtonPressed)
        {
            moveState.backward = true;
        }
        if (key == GLFW_KEY_A && leftMouseButtonPressed)
        {
            moveState.left = true;
        }
        if (key == GLFW_KEY_D && leftMouseButtonPressed)
        {
            moveState.right = true;
        }
        if (key == GLFW_KEY_Q && leftMouseButtonPressed)
        {
            moveState.down = true;
        }
        if (key == GLFW_KEY_E && leftMouseButtonPressed)
        {
            moveState.up = true;
        }
    }
    if (action == GLFW_RELEASE)
    {
        if (key == GLFW_KEY_W && leftMouseButtonPressed)
        {
            moveState.forward = false;
        }
        if (key == GLFW_KEY_S && leftMouseButtonPressed)
        {
            moveState.backward = false;
        }
        if (key == GLFW_KEY_A && leftMouseButtonPressed)
        {
            moveState.left = false;
        }
        if (key == GLFW_KEY_D && leftMouseButtonPressed)
        {
            moveState.right = false;
        }
        if (key == GLFW_KEY_Q && leftMouseButtonPressed)
        {
            moveState.down = false;
        }
        if (key == GLFW_KEY_E && leftMouseButtonPressed)
        {
            moveState.up = false;
        }
    }
}

static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        leftMouseButtonPressed = true;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        leftMouseButtonPressed = false;
        memset(&moveState, 0, sizeof(moveState));
        firstMouse = true;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

static void cursor_position_callback(GLFWwindow *window, double xpos, double ypos)
{
    if (leftMouseButtonPressed)
    {
        if (firstMouse)
        {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }
        xoffset = lastX - xpos;
        yoffset = lastY - ypos;
        lastX = xpos;
        lastY = ypos;
    }
}

void SetupCallback(GLFWwindow *window)
{
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
}