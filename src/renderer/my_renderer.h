#pragma once
#include "scene_rendering.h"
class ViewFamily;
class MySceneRenderer : public SceneRenderer
{
public:
    MySceneRenderer(const ViewFamily *InViewFamily);
};