#include "engine/classes/components/static_mesh_component.h"
#include "engine/scene_view.h"
#include "renderer/scene_private.h"
#include "renderer/renderer_module.h"
#include "renderer/scene_rendering.h"
#include "RHI/dynamic_rhi.h"
#include "RHI/RHI.h"
#include "RHI/RHICommandList.h"
#include "vk/viewport.h"
#include <GLFW/glfw3.h>

/// @name temp
/// {
#define WIDTH 1600
#define HEIGHT 1200
void OnSizeChanged(GLFWwindow *window, int width, int height);
void load_wavefront_static_mesh(std::string inputfile, StaticMesh &staticMesh);
extern VulkanViewport *drawingViewport;
/// } 

RendererModule module;
int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    // 固定窗口大小
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Xi", nullptr, nullptr);
    glfwSetFramebufferSizeCallback(window, OnSizeChanged);

    RHIInit();
    RHICommandListImmediate &dummy = RHICommandListExecutor::GetImmediateCommandList();
    dummy.SwitchPipeline(RHIPipeline::Graphics);
    {
        // 加载模型
        auto staticMesh = std::make_shared<StaticMesh>();
        load_wavefront_static_mesh("assets/cornell-box/cornell-box.obj", *staticMesh);
        auto component = std::make_shared<StaticMeshComponent>();
        component->SetStaticMesh(staticMesh);

        Scene scene;
        scene.AddPrimitive(std::static_pointer_cast<PrimitiveComponent>(component));
        std::vector<PrimitiveComponent *> primitivesView;
        for (auto &primitive : scene.primitives)
        {
            primitivesView.push_back(primitive.get());
        }
        PrimitiveComponent::AddStaticMeshes(dummy, &scene, primitivesView, true);

        CommandContext *context = GetDefaultContext();
        std::shared_ptr<VulkanViewport> viewport = CreateViewport(window, WIDTH, HEIGHT, false, PixelFormat::PF_B8G8R8A8);
        drawingViewport = viewport.get();

        // 设置相机。一个ViewFamily代表一个相机，一个相机有多个视口（View）
        SceneView view;
        view.FOV = 39.3077;
        view.NearClippingDistance = 800;
        view.farClippingDistance = 2000;
        view.ViewLocation = Vec3(278, 273, -800);
        view.ViewRect = IntVec2(WIDTH, HEIGHT);
        view.ViewRotation = Mat4(1);
        view.WorldToMetersScale = 1;

        CameraInfo viewFamily(ViewFamily{});
        viewFamily.renderTarget = drawingViewport->GetBackBuffer().get();
        viewFamily.views.push_back(&view);
        viewFamily.scene = &scene;
        TextureCreateDesc texDesc =
            TextureCreateDesc::Create2D("Base Color", view.ViewRect.x, view.ViewRect.y, PF_B8G8R8A8)
                .SetInitialState(Access::RTV | Access::SRVGraphics)
                .SetFlags(TexCreate_RenderTargetable);
        viewFamily.GetSceneTextures().GBufferA = CreateTexture(dummy, texDesc);
        viewFamily.GetSceneTextures().GBufferB = CreateTexture(dummy, texDesc.SetFormat(PF_B8G8R8A8).SetDebugName("Normal"));
        viewFamily.GetSceneTextures().GBufferC = CreateTexture(dummy, texDesc.SetFormat(PF_FloatRGB).SetDebugName("Position"));
        viewFamily.GetSceneTextures().GBufferD = CreateTexture(dummy, texDesc.SetFormat(PF_B8G8R8A8).SetDebugName("Roughness-Metallic"));

        // Loop
        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();
            context->BeginDrawingViewport(viewport);
            context->BeginFrame();

            module.BeginRenderingViewFamily(&viewFamily);

            context->EndFrame();
            context->EndDrawingViewport(viewport.get(), false);
        }
    }
    RHIExit();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}