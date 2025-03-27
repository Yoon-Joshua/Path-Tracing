#pragma once

#include "definitions.h"

class VisibilityTaskData;
class Scene;
class ViewInfo;

class VisibilityViewPacket
{
public:
    VisibilityViewPacket(VisibilityTaskData &TaskData, Scene &InScene, ViewInfo &InView, int32 ViewIndex);
};