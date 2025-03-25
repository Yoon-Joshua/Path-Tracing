#include "camera.h"
#include "simple_renderer.h"
#include <cmath>

PerCameraParameters Camera::GetCameraParameters() const
{
    Vec3 forward(sinf(M_PI_2 - pitch) * sinf(yaw),
                 cosf(M_PI_2 - pitch),
                 sinf(M_PI_2 - pitch) * cosf(yaw));
    Vec3 up(sinf(pitch) * sinf(yaw + M_PI),
            cosf(pitch),
            sinf(pitch) * cosf(yaw + M_PI));
    PerCameraParameters parameters;
    parameters.view = Lookat(position, position + forward, up);
    parameters.proj = Perspective(Radians(60), (float)WIDTH / (float)HEIGHT, far, near);
    return parameters;
}