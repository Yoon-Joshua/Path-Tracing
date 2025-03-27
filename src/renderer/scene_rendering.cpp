#include "engine/scene_interface.h"
#include "engine/scene_view.h"
#include "engine/classes/components/primitive_component.h"
#include "render_core/static_states.h"
#include "RHI/RHICommandList.h"
#include "RHI/RHIContext.h"
#include "RHI/dynamic_rhi.h"
#include "scene_rendering.h"
#include "renderer_module.h"
#include "scene_private.h"
#include "scene_visibility.h"
#include "my_renderer.h"
#include <memory>
#include <array>

CameraInfo::CameraInfo(const ViewFamily &InViewFamily) : ViewFamily(InViewFamily)
{
    bIsViewFamilyInfo = true;
}

SceneRenderer::SceneRenderer(const ViewFamily *InViewFamily) : camera(*InViewFamily)
{
    AllViews.resize(InViewFamily->views.size());
    for (auto &viewInfo : AllViews)
    {
        viewInfo = new ViewInfo;
    }
}

SceneRenderer::~SceneRenderer()
{
    for (auto &viewInfo : AllViews)
    {
        delete viewInfo;
    }
}

void SceneRenderer::CreateSceneRenderers(std::vector<const ViewFamily *> inViewFamilies, std::vector<SceneRenderer *> &outSceneRenderers)
{
    check(outSceneRenderers.size() == 0);

    if (!inViewFamilies.size())
        return;

    const SceneInterface *scene = inViewFamilies[0]->scene;
    check(scene);
    for (int32 familyIndex = 0; familyIndex < inViewFamilies.size(); familyIndex++)
    {
        const ViewFamily *inViewFamily = inViewFamilies[familyIndex];
        check(inViewFamily);
        check(inViewFamily->scene == scene);
        outSceneRenderers.push_back(new MySceneRenderer(inViewFamily));
    }
}

void SceneRenderer::BeginInitViews(IVisibilityTaskData *visibilityTaskData)
{
    check(0);
}

void SceneRenderer::BeginInitViews(ExclusiveDepthStencil::Type BasePassDepthStencilAccess)
{
    std::vector<ViewCommands> ViewCommandsPerView(Views.size());
    ComputeViewVisibility(BasePassDepthStencilAccess, ViewCommandsPerView);
}

void SceneRenderer::EndInitViews() {}

void SceneRenderer::GatherDynamicMeshElements(std::vector<ViewInfo> &InViews,
                                              const Scene *InScene) {}

void SceneRenderer::SetupMeshPass(ViewInfo &View, ExclusiveDepthStencil::Type BasePassDepthStencilAccess, ViewCommands &ViewCommands)
{
    for (int32 PassIndex = 0; PassIndex < MeshPass::Num; PassIndex++)
    {
        const MeshPass::Type PassType = (MeshPass::Type)PassIndex;
        MeshPassProcessor *meshPassProcessor = nullptr;
        MeshDrawCommandPass pass = View.MeshDrawCommandPasses[PassIndex];

        pass.DispatchPassSetup(scene, &View, PassType, BasePassDepthStencilAccess, meshPassProcessor,
                               View.DynamicMeshElements,
                               &View.DynamicMeshElementsPassRelevance,
                               View.NumVisibleDynamicMeshElements[PassIndex],
                               ViewCommands.DynamicMeshCommandBuildRequests[PassIndex],
                               ViewCommands.NumDynamicMeshCommandBuildRequestElements[PassIndex],
                               ViewCommands.MeshCommands[PassIndex]);
    }
}

void SceneRenderer::Render(RHICommandListImmediate &RHICmdList)
{
    IVisibilityTaskData *visibilityTaskData = OnRenderBegin(RHICmdList);
    //  CommitFinalPipelineState
    //  GSystemTextures.InitializeTextures
    //  FSceneTextures::InitializeViewFamily
    // BeginInitViews(VisibilityTaskData);
    // VisibilityTaskData->FinishGatherDynamicMeshElements();
    //  EndInitViews

    const ExclusiveDepthStencil::Type BasePassDepthStencilAccess = ExclusiveDepthStencil::DepthWrite_StencilWrite;

    // 初始化视图：进行剔除，计算相关性
    {
        for (ViewInfo &viewInfo : Views)
        {
            viewInfo.primitiveVisibilityMap.assign(scene->primitives.size(), true);
        }

        // BeginInitVisibility
        // LightVisibility
        // FrustumCull
        // OcclusionCull

        // Compute Relvance

        // GDME - Gather Dynamic Mesh Elements

        // SetupMeshPasses
        for (ViewInfo &viewInfo : Views)
        {
            viewInfo.DynamicMeshElementsPassRelevance.resize(viewInfo.DynamicMeshElements.size());
        }

        for (ViewInfo &viewInfo : Views)
        {
            ViewCommands viewCommands;
            viewCommands.MeshCommands;
            SetupMeshPass(viewInfo, BasePassDepthStencilAccess, viewCommands);
        }
    }

    GraphicsPipelineStateInitializer graphicsPSOInit;
    graphicsPSOInit.RenderTargetsEnabled = 1;
    graphicsPSOInit.RenderTargetFormats[0] = camera.renderTarget->GetDesc().Format;
    graphicsPSOInit.RenderTargetFlags[0] = camera.renderTarget->GetDesc().Flags;
    graphicsPSOInit.NumSamples = 1;
    graphicsPSOInit.DepthStencilTargetFormat = PF_Unknown;

    graphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI().get();
    graphicsPSOInit.DepthStencilState = TStaticDepthStencilState<>::GetRHI().get();

    graphicsPSOInit.SubpassHint = SubpassHint::None;
    graphicsPSOInit.SubpassIndex = 0;

    graphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None>::GetRHI().get();
    graphicsPSOInit.PrimitiveType = PT_TriangleList;

    // 遍历所有相机视口
    for (int32 index = 0; index < camera.views.size(); ++index)
    {
        auto context = GetDefaultContext();
    }
}

/// @todo 未完成 太难了
IVisibilityTaskData *SceneRenderer::OnRenderBegin(RHICommandListImmediate &RHICmdList)
{
    IVisibilityTaskData *visibilityTaskData = nullptr;

    Scene::UpdateParameters sceneUpdateParameters;
    sceneUpdateParameters.callbacks.postStaticMeshUpdate = [&](std::function<void()> StaticMeshUpdateTask)
    {
        visibilityTaskData = LaunchVisibilityTasks(RHICmdList, *this, StaticMeshUpdateTask);
    };

    scene->Update(sceneUpdateParameters);

    return visibilityTaskData;
}

void SceneRenderer::RenderBegin(RHICommandListImmediate &RHICmdList, const std::vector<SceneRenderer *> &SceneRenderers)
{
    // printf("Render Begin\n");
}
void SceneRenderer::RenderEnd(RHICommandListImmediate &RHICmdList, const std::vector<SceneRenderer *> &SceneRenderers)
{
    // printf("Render End\n");
}

/// Helper function performing actual work in render thread.
/// @param SceneRenderers	List of scene renderers to use for rendering.
static void RenderViewFamilies(RHICommandListImmediate &RHICmdList, const std::vector<SceneRenderer *> &SceneRenderers)
{
    // All renderers point to the same Scene (calling code asserts this)
    Scene *const Scene = SceneRenderers[0]->scene;

    SceneRenderer::RenderBegin(RHICmdList, SceneRenderers);
    for (SceneRenderer *sceneRenderer : SceneRenderers)
    {
        sceneRenderer->Render(RHICmdList);
    }
    SceneRenderer::RenderEnd(RHICmdList, SceneRenderers);
}

void RendererModule::BeginRenderingViewFamily(ViewFamily *viewFamily)
{
    Scene *const scene = (Scene *)viewFamily->scene;

    if (scene)
    {
        std::vector<const ViewFamily *> viewFamiliesConst = {viewFamily};
        // Construct the scene renderers.  This copies the view family attributes into its own structures.
        std::vector<SceneRenderer *> sceneRenderers;
        SceneRenderer::CreateSceneRenderers(viewFamiliesConst, sceneRenderers);
        RenderViewFamilies(GRHICommandListExecutor.GetImmediateCommandList(), sceneRenderers);

        // RHICommandListExecutor::GetImmediateCommandList().ImmediateFlush(EImmediateFlushType::FlushRHIThreadFlushResources);
    }
}
