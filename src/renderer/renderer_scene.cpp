#include "scene_private.h"

#include <memory>

Scene::Scene() : GPUScene(*this) {}

Scene::~Scene() {}

void Scene::Update(const UpdateParameters &parameters)
{
    std::function<void ()> addStaticMeshesTask = []() -> void {};
    parameters.callbacks.postStaticMeshUpdate(addStaticMeshesTask);
}

void Scene::AddPrimitive(std::shared_ptr<PrimitiveComponent> primitive)
{
    primitives.push_back(primitive);
}

void Scene::RemovePrimitive(std::shared_ptr<PrimitiveComponent> primitive)
{
    check(0);
}