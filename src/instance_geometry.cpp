#include "bsp/instance_geometry.hpp"
#include <limits>
#include <new>
#include <utility>

namespace bsp {
HRESULT create_generated_instance_geometry_00b4c8d0_fragment(
    D3D9StateCache& states,
    const std::shared_ptr<VertexBufferBinding>& renderer_shared_vertices,
    const GeneratedInstanceGeometrySource& source,
    const GeneratedInstanceGeometryGenerator& generator,
    const GeneratedInstanceMaterialClone& material_clone,
    std::shared_ptr<GeneratedInstanceGeometry>& output) {
    if (!renderer_shared_vertices || !renderer_shared_vertices->buffer
        || renderer_shared_vertices->flags != generated_instance_stream_flags
        || renderer_shared_vertices->capacity != generated_instance_shared_vertex_capacity
        || !source.selected_mesh_stream || !source.selected_mesh_stream->declaration
        || !generator.instance_declaration || !generator.instance_declaration->stride
        || !generator.combined_layout || !material_clone.owner)
        return E_INVALIDARG;

    // Verify that the supplied owner is the real shared dynamic allocation,
    // rather than accepting metadata around a private MANAGED buffer.
    D3DVERTEXBUFFER_DESC description{};
    const HRESULT described = renderer_shared_vertices->buffer->GetDesc(&description);
    if (FAILED(described)) return described;
    if (description.Size != generated_instance_shared_vertex_capacity
        || description.Pool != D3DPOOL_DEFAULT
        || description.Usage != (D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY)
        || description.FVF != 0)
        return E_INVALIDARG;

    try {
        auto geometry = std::make_shared<GeneratedInstanceGeometry>();
        //00b4bc00 registers the dynamic stream on the shared physical owner.
        // Its raw registry cannot keep the stream alive. The custom deleter
        // unregisters only when all geometry/cache owners have released it.
        auto stream = std::shared_ptr<LogicalVertexStream>(new LogicalVertexStream,
            [&states](LogicalVertexStream* value) {
                if (value->physical)
                    states.unregister_vertex_stream_00b4b3f0(*value->physical, *value);
                delete value;
            });
        stream->physical = renderer_shared_vertices;
        stream->declaration = generator.instance_declaration;
        stream->offset = (std::numeric_limits<UINT>::max)();
        stream->vertex_count = 0;
        stream->flags = generated_instance_stream_flags;
        stream->tag = generated_instance_stream_tag;
        states.register_logical_stream_00b4b1e0(*renderer_shared_vertices, *stream);

        geometry->mesh_stream = source.selected_mesh_stream;
        geometry->instance_stream = std::move(stream);
        geometry->indices = source.indices;
        geometry->combined_layout = generator.combined_layout;
        geometry->section.primitive = source.primitive;
        geometry->section.range_words = source.range_words;
        //00b4ca0c..00b4ca27 copies only these five DWORDs. Fresh section+1Ch
        // and +34h remain zero, regardless of values in the source section.
        geometry->section.material_order = material_clone.material_order;
        geometry->section.material_queue_index = material_clone.material_queue_index;
        geometry->section.material_clone_owner = material_clone.owner;
        output = std::move(geometry);
        return S_OK;
    } catch (const std::bad_alloc&) {
        return E_OUTOFMEMORY;
    }
}
}
