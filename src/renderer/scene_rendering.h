#pragma once
#include <vector>
#include "engine/scene_interface.h"
#include "renderer/internal/scene_textures.h"
#include "renderer/mesh_draw_commands.h"
#include "renderer/mesh_pass_processor.h"
#include "engine/mesh_batch.h"
#include "engine/scene_view.h"
#include "RHI/RHICommandList.h"

#include "scene_visibility.h"

class ViewCommands;
class IVisibilityTaskData;

/// View family plus associated transient scene textures.
/// 渲染模块
class CameraInfo : public ViewFamily
{
public:
    explicit CameraInfo(const ViewFamily &InViewFamily);

    inline SceneTextures &GetSceneTextures() { return sceneTextures; }

private:
    SceneTextures sceneTextures;
};

class ViewInfo : public SceneView
{
public:
    std::array<MeshDrawCommandPass, MeshPass::Num> MeshDrawCommandPasses;

    /** Gathered in initviews from all the primitives with dynamic view relevance, used in each mesh pass. */
    std::vector<MeshBatchAndRelevance> DynamicMeshElements;

    /* Mesh pass relevance for gathered dynamic mesh elements. */
    std::vector<MeshPassMask> DynamicMeshElementsPassRelevance;

    /** Number of dynamic mesh elements per mesh pass (inside FViewInfo::DynamicMeshElements). */
    int32 NumVisibleDynamicMeshElements[MeshPass::Num];

    /** A map from static mesh ID to a boolean visibility value. */
    std::vector<bool> StaticMeshVisibilityMap;

    /** A map from primitive ID to a boolean visibility value. */
    std::vector<bool> primitiveVisibilityMap;
};

/// Used as the scope for scene rendering functions.
/// It is initialized in the game thread by FSceneViewFamily::BeginRender, and then passed to the rendering thread.
/// The rendering thread calls Render(), and deletes the scene renderer when it returns.
class SceneRenderer : public SceneInterface
{
public:
    /******************** 润 **********************/
    VisibilityTaskData *allocateVisibilityTaskData(RHICommandListImmediate &RHICmdList, SceneRenderer &SceneRenderer)
    {
        visibilityTaskData = std::make_shared<VisibilityTaskData>(RHICmdList, SceneRenderer);
        return visibilityTaskData.get();
    }
    std::shared_ptr<VisibilityTaskData> visibilityTaskData = nullptr;
    /**********************************************/
    Scene *scene = nullptr;

    /** The view family being rendered.  This references the Views array. */
    CameraInfo camera;

    /** The views being rendered. */
    std::vector<ViewInfo> Views;

    SceneRenderer(const ViewFamily *InViewFamily);
    ~SceneRenderer();
    /// @brief 创建一个SceneRenderer
    static void CreateSceneRenderers(std::vector<const ViewFamily *>, std::vector<SceneRenderer *> &);
    static void RenderBegin(RHICommandListImmediate &RHICmdList, const std::vector<SceneRenderer *> &SceneRenderers);
    static void RenderEnd(RHICommandListImmediate &RHICmdList, const std::vector<SceneRenderer *> &SceneRenderers);

    /** Determines which primitives are visible for each view. */
    void BeginInitViews(IVisibilityTaskData *VisibilityTaskData);
    void BeginInitViews(ExclusiveDepthStencil::Type);
    void EndInitViews();
    void ComputeViewVisibility(ExclusiveDepthStencil::Type, std::vector<ViewCommands> &);
    void GatherDynamicMeshElements(std::vector<ViewInfo> &InViews, const Scene *InScene);

    void Render(RHICommandListImmediate &RHICmdList);

    void SetupMeshPass(ViewInfo &View, ExclusiveDepthStencil::Type BasePassDepthStencilAccess, ViewCommands &ViewCommands);

    /** Called at the begin / finish of the scene render. */
    IVisibilityTaskData *OnRenderBegin(RHICommandListImmediate &RHICmdList);

    /** All views include main camera views and custom render pass views. */
    std::vector<ViewInfo *> AllViews;
};