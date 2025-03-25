#pragma once

#include "simple_scene.h"
#include "camera.h"
#include "RHI/RHICommandList.h"
#include <vector>

class SimpleStaticMesh;
class CommandContext;
class SimpleScene;

struct PerCameraParameters
{
    Mat4 view;
    Mat4 proj;
};

struct PerObjectParameters
{
    Mat4 local_to_world;
};

class SimpleRenderer
{
public:
    SimpleRenderer(SimpleScene &s);
    void Render(CommandContext *context, SimpleScene &scene, Viewport *viewport);

    void Prepare(RHICommandListBase &immediate);
    void Update(RHICommandListBase &immediate, Camera &);

private:
    SimpleScene &scene;
    std::shared_ptr<UniformBuffer> perCamera;
    std::shared_ptr<Texture> depth;
};