#pragma once
#include "core/math/vec.h"

#include "interaction.h"
#include "application.h"

#define DEFAULT_SPEED 0.5
#define DEFAULT_FAR 100;
#define DEFAULT_NEAR 0.1

struct CameraInfo
{
    Mat4 model;
    Mat4 view;
    Mat4 proj;
};

class Camera
{
private:
    Vec3 position;
    float yaw;
    float pitch;
    float far = DEFAULT_FAR;
    float near = DEFAULT_NEAR;

    float speed;
    float sensitivity = 0.005;

public:
    Camera(Vec3 pos = Vec3(0, 0, 0), float yaw = 0, float pitch = 0, float speed = DEFAULT_SPEED)
        : position(pos), yaw(yaw), pitch(pitch), speed(speed) {}

    inline void Update(float deltaTime)
    {
        if (moveState.IsMoving())
        {
            Vec3 moveDirection;
            Vec3 forward(sinf(M_PI_2 - pitch) * sinf(yaw),
                         cosf(M_PI_2 - pitch),
                         sinf(M_PI_2 - pitch) * cosf(yaw));
            Vec3 up(sinf(pitch) * sinf(yaw + M_PI),
                    cosf(pitch),
                    sinf(pitch) * cosf(yaw + M_PI));
            Vec3 right = Cross(forward, up);
            if (moveState.forward)
                moveDirection = forward;
            else if (moveState.backward)
                moveDirection = -forward;
            else if (moveState.left)
                moveDirection = -right;
            else if (moveState.right)
                moveDirection = right;
            else if (moveState.up)
                moveDirection = up;
            else
                moveDirection = -up;

            position = position + deltaTime * speed * moveDirection;
        }

        if (xoffset != 0 || yoffset != 0)
        {
            yaw += xoffset * sensitivity;
            pitch += yoffset * sensitivity;
            if (pitch > M_PI_2)
                pitch = M_PI_2;
            if (pitch < -M_PI_2)
                pitch = -M_PI_2;
            xoffset = 0;
            yoffset = 0;
        }
    }

    inline CameraInfo GetCameraInfo() const
    {
        Vec3 forward(sinf(M_PI_2 - pitch) * sinf(yaw),
                     cosf(M_PI_2 - pitch),
                     sinf(M_PI_2 - pitch) * cosf(yaw));
        Vec3 up(sinf(pitch) * sinf(yaw + M_PI),
                cosf(pitch),
                sinf(pitch) * cosf(yaw + M_PI));
        CameraInfo info;
        info.model = Rotate(Mat4(1), Radians(0), Vec3(0, 0, 1));
        info.view = Lookat(position, position + forward, up);
        info.proj = Perspective(Radians(60), (float)WIDTH / (float)HEIGHT, far, near);
        return info;
    }
};