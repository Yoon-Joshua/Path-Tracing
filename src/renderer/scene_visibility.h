#pragma once
#include "definitions.h"
#include "renderer/mesh_pass_processor.h"
#include "engine/static_mesh_batch.h"
class ViewCommands
{
public:
    ViewCommands()
    {
        for (int32 PassIndex = 0; PassIndex < MeshPass::Num; ++PassIndex)
        {
            NumDynamicMeshCommandBuildRequestElements[PassIndex] = 0;
        }
    }

    // 已经缓存好的静态网格
    std::array<std::vector<VisibleMeshDrawCommand>, MeshPass::Num> MeshCommands;

    // 需要缓存的静态网格（view相关）
    std::array<int32, MeshPass::Num> NumDynamicMeshCommandBuildRequestElements;
    std::array<std::vector<const StaticMeshBatch *>, MeshPass::Num> DynamicMeshCommandBuildRequests;
};

class IVisibilityTaskData
{
public:
    virtual void ProcessTasks() = 0;
};

////////////////////////////////////////////////////////////////////////// Private
class VisibilityTaskData;
class ViewInfo;
class VisibilityViewPacket;
class RHICommandListImmediate;
class SceneRenderer;

/*
此类管理与特定场景渲染器关联的所有视图的可见性计算相关的所有状态。
在并行模式下，复杂的任务图处理每个可见性阶段，并将结果从一个阶段流水线到下一个阶段。
这避免了主要的连接/分叉同步点，除了当前仅限于渲染线程的动态网格元素收集。
对于不受益于并行性或不支持并行性的平台，还支持以渲染线程为中心的模式，该模式通过一些并行来处理渲染线程上的可见性，以获得任务线程的支持。
*/
class VisibilityTaskData : public IVisibilityTaskData
{
private:
    RHICommandListImmediate &RHICmdList;
    SceneRenderer &sceneRenderer;
    Scene &scene;
    std::vector<ViewInfo *> views;
    std::vector<VisibilityViewPacket> viewPackets;

    struct DynamicMeshElements
    {
        std::vector<ViewCommands> viewCommandsPerView;
    } dynamicMeshElements;

    struct Tasks
    { // These legacy tasks are used to interface with the jobs launched prior to gather dynamic mesh elements.
    } Tasks;

public:
    VisibilityTaskData(RHICommandListImmediate &RHICmdList, SceneRenderer &SceneRenderer);
    void LaunchVisibilityTasks(std::function<void()> &BeginInitVisibilityPrerequisites);
    void ProcessTasks()
    {
    }
};

class SceneRenderer;
extern IVisibilityTaskData *LaunchVisibilityTasks(RHICommandListImmediate &RHICmdList,
                                                  SceneRenderer &SceneRenderer,
                                                  std::function<void()> &BeginInitVisibilityTaskPrerequisites);
