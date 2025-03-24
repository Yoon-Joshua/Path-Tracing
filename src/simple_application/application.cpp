#include "application.h"
#include "camera.h"
#include "simple_scene.h"
#include "simple_renderer.h"
#include "simple_timer.h"
#include "definitions.h"
#include "core/math/vec.h"
#include "RHI/RHICommandList.h"
#include "RHI/RHIResources.h"
#include "RHI/RHIContext.h"
#include "RHI/dynamic_rhi.h"

#include "GLFW/glfw3.h"
#include "RHI/dynamic_rhi.h"
#include "RHI/RHI.h"
#include "RHI/pipeline_state_cache.h"
#include "render_core/static_states.h"
#include "vk/viewport.h"

#include "stb_image.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <memory>

SimpleStaticMesh LoadWavefrontStaticMesh(std::string inputfile, RHICommandListBase &cmdList)
{
    SimpleStaticMesh staticMesh;

    tinyobj::ObjReaderConfig reader_config;
    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(inputfile, reader_config))
    {
        if (!reader.Error().empty())
        {
            std::cerr << "TinyObjReader: " << reader.Error();
        }
        exit(1);
    }

    if (!reader.Warning().empty())
    {
        std::cout << "TinyObjReader: " << reader.Warning();
    }

    auto &attrib = reader.GetAttrib();
    auto &shapes = reader.GetShapes();
    auto &mats = reader.GetMaterials();

    assert(shapes.size() == 1);

    staticMesh.LOD.resize(shapes.size());
    for (size_t s = 0; s < shapes.size(); s++)
    {
        SimpleLODResource &currentLOD = staticMesh.LOD[0];
        assert(currentLOD.index.empty());
        assert(currentLOD.position.empty());
        assert(currentLOD.normal.empty());
        assert(currentLOD.uv.empty());
        assert(currentLOD.tangent.empty());

        size_t numPolygon = shapes[s].mesh.num_face_vertices.size();

        // 1. load vertices and polygon. Remove unnecessary vertices.
        std::vector<uint32> indices;
        {
            std::unordered_map<SimpleVertex, size_t> uniqueVertices{};
            std::vector<SimpleVertex> vertices;

            size_t index_offset = 0;
            for (size_t f = 0; f < numPolygon; f++)
            {
                size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

                // only support triangles
                assert(fv == 3);

                // Loop over vertices in the face.
                for (size_t v = 0; v < fv; v++)
                {
                    SimpleVertex vertex;

                    // access to vertex
                    tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                    vertex.position.x = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                    vertex.position.y = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                    vertex.position.z = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

                    // Check if `normal_index` is zero or positive. negative = no normal
                    // data
                    if (idx.normal_index >= 0)
                    {
                        vertex.normal.x = attrib.normals[3 * size_t(idx.normal_index) + 0];
                        vertex.normal.y = attrib.normals[3 * size_t(idx.normal_index) + 1];
                        vertex.normal.z = attrib.normals[3 * size_t(idx.normal_index) + 2];
                    }

                    // Check if `texcoord_index` is zero or positive. negative = no texcoord data
                    if (idx.texcoord_index >= 0)
                    {
                        vertex.texCoord.x = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
                        vertex.texCoord.y = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
                    }
                    // 去除重复顶点
                    if (uniqueVertices.count(vertex) == 0)
                    {
                        uniqueVertices[vertex] = vertices.size();
                        vertices.push_back(vertex);
                    }
                    indices.push_back(uniqueVertices[vertex]);
                }
                index_offset += fv;
            }
            // 各个属性分开存储
            for (SimpleVertex &v : vertices)
            {
                currentLOD.position.push_back(v.position);
                currentLOD.normal.push_back(v.normal);
                currentLOD.uv.push_back(v.texCoord);
            }
        }
        // 2. Load materials
        {
            staticMesh.materials.resize(mats.size());
            for (size_t i = 0; i < mats.size(); ++i)
            {
                SimpleMaterial &material = staticMesh.materials[i];
                material.name = mats[i].name;
                material.albedo =
                    Vec3(mats[i].diffuse[0], mats[i].diffuse[1], mats[i].diffuse[2]);
                material.emission = glm::vec3(mats[i].emission[0], mats[i].emission[1],
                                              mats[i].emission[2]);
                material.metallic = mats[i].metallic;
                material.roughness = mats[i].roughness;

                if (material.emission.x > 0 || material.emission.y > 0 ||
                    material.emission.z > 0)
                {
                    material.type = SimpleMaterial::MaterialType::EMISSIVE;
                }
                else if (material.metallic > 0 || material.roughness > 0)
                {
                    material.type = SimpleMaterial::MaterialType::SPECULAR;
                }
                else
                {
                    material.type = SimpleMaterial::MaterialType::DIFFUSE;
                }
            }
        }
        // 3. process sections
        {
            // map material ID to section ID
            std::unordered_map<size_t, size_t> mat2Section;
            std::vector<std::vector<uint32>> section2Polygons;

            for (size_t f = 0; f < numPolygon; f++)
            {
                if (mat2Section.count(shapes[s].mesh.material_ids[f]) == 0)
                {
                    size_t sectionID = currentLOD.sections.size();
                    currentLOD.sections.push_back(SimpleSection());
                    section2Polygons.push_back(std::vector<uint32>());
                    currentLOD.sections[sectionID].materialIndex = shapes[s].mesh.material_ids[f];
                    section2Polygons[sectionID].push_back(f);
                    mat2Section[shapes[s].mesh.material_ids[f]] = sectionID;
                }
                else
                {
                    size_t sectionID = mat2Section[shapes[s].mesh.material_ids[f]];
                    section2Polygons[sectionID].push_back(f);
                }
            }

            // 重排index，使得相同材质的polygon挨在一起
            for (int sectionID = 0; sectionID < section2Polygons.size(); ++sectionID)
            {
                auto &polygonIDs = section2Polygons[sectionID];

                currentLOD.sections[sectionID].firstIndex = currentLOD.index.size();
                currentLOD.sections[sectionID].numTriangles = polygonIDs.size();
                for (auto polygonID : polygonIDs)
                {
                    size_t fv = size_t(shapes[s].mesh.num_face_vertices[polygonID]);
                    assert(fv == 3);

                    currentLOD.index.push_back(indices[polygonID * fv + 0]);
                    currentLOD.index.push_back(indices[polygonID * fv + 1]);
                    currentLOD.index.push_back(indices[polygonID * fv + 2]);
                }
            }
        }

        // 4. upload to GPU
        auto Upload = [](RHICommandListBase &cmdList, void *src, uint32 elementNum, uint32 stride, std::shared_ptr<Buffer> &dst, BufferUsageFlags flag)
        {
            ResourceCreateInfo ci{};
            uint32 size = elementNum * stride;
            BufferDesc bufferDesc(size, stride, flag);
            dst = CreateBuffer(bufferDesc, Access::VertexOrIndexBuffer, ci);
            void *mapped = LockBuffer_BottomOfPipe(cmdList, dst.get(), 0, size, ResourceLockMode::RLM_WriteOnly);
            memcpy(mapped, src, size);
            UnlockBuffer_BottomOfPipe(cmdList, dst.get());
        };
        Upload(cmdList, currentLOD.position.data(), currentLOD.position.size(), sizeof(Vec3), currentLOD.positonBuffer, BUF_VertexBuffer);
        Upload(cmdList, currentLOD.normal.data(), currentLOD.normal.size(), sizeof(Vec3), currentLOD.normalBuffer, BUF_VertexBuffer);
        Upload(cmdList, currentLOD.uv.data(), currentLOD.uv.size(), sizeof(Vec2), currentLOD.uvBuffer, BUF_VertexBuffer);
        Upload(cmdList, currentLOD.index.data(), currentLOD.index.size(), sizeof(uint32), currentLOD.indexBuffer, BUF_IndexBuffer);
    }

    return staticMesh;
}

const int WIDTH = 1600;
const int HEIGHT = 1200;
extern Viewport *drawingViewport;
extern std::vector<uint8> process_shader(std::string filename, ShaderFrequency freq);

void RunSimpleApplication(GLFWwindow *window)
{
    SimpleTimer timer;

    RHICommandListBase &immediate = RHICommandListExecutor::GetImmediateCommandList();
    immediate.SwitchPipeline(RHIPipeline::Graphics);

    SimpleScene scene;
    SimpleRenderer renderer(scene);
    Camera camera;
    scene.AddStaticMesh(LoadWavefrontStaticMesh("assets/cube.obj", immediate));

    CommandContext *context = GetDefaultContext();
    std::shared_ptr<Viewport> viewport = CreateViewport(window, WIDTH, HEIGHT, false, PixelFormat::PF_B8G8R8A8);
    drawingViewport = viewport.get();

    renderer.Prepare(immediate);
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        double deltaTime = timer.tick();
        camera.Update(deltaTime);
        renderer.Update(immediate,camera);

        context->BeginDrawingViewport(viewport);
        context->BeginFrame();

        renderer.Render(context, scene, camera, viewport.get());

        context->EndFrame();
        context->EndDrawingViewport(viewport.get(), false);
    }
    context->SubmitCommandsHint();
    return;
}