/** An enumeration of the different RHI reference types. */
enum ERHIResourceType
{
    RRT_None,

    RRT_SamplerState,
    RRT_RasterizerState,
    RRT_DepthStencilState,
    RRT_BlendState,
    RRT_VertexDeclaration,
    RRT_VertexShader,
    RRT_MeshShader,
    RRT_AmplificationShader,
    RRT_PixelShader,
    RRT_GeometryShader,
    RRT_RayTracingShader,
    RRT_ComputeShader,
    RRT_GraphicsPipelineState,
    RRT_ComputePipelineState,
    RRT_RayTracingPipelineState,
    RRT_BoundShaderState,
    RRT_UniformBufferLayout,
    RRT_UniformBuffer,
    RRT_Buffer,
    RRT_Texture,
    // @todo: texture type unification - remove these
    RRT_Texture2D,
    RRT_Texture2DArray,
    RRT_Texture3D,
    RRT_TextureCube,
    // @todo: texture type unification - remove these
    RRT_TextureReference,
    RRT_TimestampCalibrationQuery,
    RRT_GPUFence,
    RRT_RenderQuery,
    RRT_RenderQueryPool,
    RRT_Viewport,
    RRT_UnorderedAccessView,
    RRT_ShaderResourceView,
    RRT_RayTracingAccelerationStructure,
    RRT_StagingBuffer,
    RRT_CustomPresent,
    RRT_ShaderLibrary,
    RRT_PipelineBinaryLibrary,
    RRT_ShaderBundle,

    RRT_Num
};

class RHIResource
{
public:
    RHIResource(ERHIResourceType t) {}
};

class RHITexture : public RHIResource
{
};