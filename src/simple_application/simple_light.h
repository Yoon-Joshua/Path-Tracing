#pragma once
#include "core/math/vec.h"

struct LightParameters
{
    Vec3 position;
    uint32 type;
    Vec3 direction;
    float padding0;
    Vec3 intensity;
    float padding1;
};

class SimpleLight
{
public:
    enum class LightType : uint32
    {
        DIRECTIONAL = 0,
        POINT = 1
    } type;

    SimpleLight() : type(LightType::DIRECTIONAL) {}

    LightParameters GetLightParamters() const
    {
        LightParameters parameters = {};
        parameters.position = position;
        parameters.type = (uint32)type;
        parameters.direction = direction;
        parameters.intensity = direction;
        return parameters;
    }

private:
    Vec3 position;
    Vec3 direction;
    Vec3 intensity;
};
