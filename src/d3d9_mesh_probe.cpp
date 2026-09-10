// One installed-model diagnostic. Scene/camera inputs are explicit host inputs;
// no native world, shadow-buffer generation or gameplay is implied by this draw.
#include "asset_stream_probe.hpp"
#include "installed_model_probe.hpp"
#include "bsp/compiled_material.hpp"
#include "bsp/resource_path.hpp"
#include "bsp/vertex_format.hpp"
#include "bsp/mesh_gpu_streams.hpp"
#include "bsp/mesh_decode_bindings.hpp"
#include "bsp/building_instance.hpp"
#include "bsp/material_lighting.hpp"
#include "bsp/material_constants.hpp"
#include "bsp/material_textures.hpp"
#include "bsp/d3d9_texture.hpp"
#include "bsp/texture_load_policy.hpp"
#include "bsp/instance_grouping.hpp"
#include "bsp/instance_geometry.hpp"
#include "bsp/material_clone.hpp"
#include "bsp/d3d9_resources.hpp"
#include <algorithm>
#include <cstring>
#include <set>
#include <cstdio>
#include <fstream>
#include <filesystem>

namespace {
template<class T> struct OwnedCom {
    T* p{};
    ~OwnedCom() { if (p) p->Release(); }
    OwnedCom() = default;
    OwnedCom(const OwnedCom&) = delete;
    OwnedCom& operator=(const OwnedCom&) = delete;
};
struct TextureImports {
    HMODULE module = LoadLibraryExW(L"d3dx9_40.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    bsp::ReadImageInfoFromMemory info{};
    bsp::CreateTextureFromMemory create{};
    TextureImports() {
        if (!module) return;
        auto address = GetProcAddress(module, "D3DXGetImageInfoFromFileInMemory");
        std::memcpy(&info, &address, sizeof(info));
        address = GetProcAddress(module, "D3DXCreateTextureFromFileInMemoryEx");
        std::memcpy(&create, &address, sizeof(create));
    }
    ~TextureImports() { if (module) FreeLibrary(module); }
};

bool load_texture(IDirect3DDevice9& device, AssetStreamProbe& assets,
    const TextureImports& imports, const std::string& requested,
    std::unique_ptr<bsp::D3D9RetainedTexture2D>& output, std::string& error) {
    std::shared_ptr<bsp::MemoryStream> source;
    auto name = requested;
    bsp::lowercase_resource_name_004bcc00(name);
    std::string logical;
    if (!imports.info || !imports.create || !assets.read(name, source, error, &logical)) return false;
    const auto result = bsp::load_retained_texture_2d_00b2c2d0_fragment(device,
        imports.info, imports.create, source,
        {static_cast<std::uint32_t>(logical.size()), logical.c_str()}, 0, output);
    std::printf("Mounted mesh texture: %s -> %s hr=0x%08lx\n", requested.c_str(), logical.c_str(),
        static_cast<unsigned long>(result));
    return SUCCEEDED(result) && output && output->texture();
}

bool scene_constants(const std::vector<bsp::ReflectedShaderConstant>& reflection,
    std::vector<float>& words, bool vertex, std::string& error) {
    // Deliberate host scene: fixed orthographic camera, uniform ambient cube,
    // one directional light, no fog/time effect. These are explicit scene
    // inputs, not guessed native initialization of the world or renderer.
    const float view_projection[]{.24f,-.14f,-.1f,0, 0,.28f,-.1f,0,
        -.24f,-.14f,-.1f,0, 0,0,.5f,1};
    const float screen_texture[]{.5f,0,0,0, 0,-.5f,0,0, 0,0,1,0, .5f,.5f,0,1};
    for (const auto& constant : reflection) {
        if (constant.register_set == 3) continue;
        if (constant.register_set != 2 || constant.parameter_type != 3
            || constant.register_index + constant.register_count > words.size()/4) return false;
        auto* value = words.data() + 4 * constant.register_index;
        std::fill(value, value + constant.register_count * 4, 0.0f);
        const auto& name = constant.name;
        if (name == "cVtxElemScale" || name == "cVtxElemOffset") continue; // Recovered binder below.
        if (vertex && (name == "cViewProjMat" || name == "cScreenToTextureMat")) {
            if (constant.rows != 4 || constant.columns != 4 || constant.register_count != 4) return false;
            bsp::write_system_matrix_00b404a0(value,
                name == "cViewProjMat" ? view_projection : screen_texture);
        } else if (vertex && name == "cAmbientCube" && constant.register_count == 6) {
            for (unsigned i = 0; i < 6; ++i) {
                value[i*4] = value[i*4+1] = value[i*4+2] = .35f; value[i*4+3] = 1;
            }
        } else if (vertex && name == "cDirLightDiffuseColor") {
            value[0] = value[1] = value[2] = .65f; value[3] = 1;
        } else if (vertex && name == "cDirLightWorldSpaceDir") {
            value[0] = value[1] = value[2] = .577350269f;
        } else if (vertex && name == "cWorldSpaceEyePos") {
            value[0] = value[1] = value[2] = 8;
        } else if (vertex && name == "cFogParams") {
            value[0] = 100; value[1] = 200;
        } else if (vertex && name == "cFogHeightParams") value[0] = 1;
        else if ((vertex && (name == "cFogColor" || name == "cFogDirColor4" || name == "cPointLightsData"))
            || (!vertex && name == "cElapsedTime")) {}
        else { error = "Unresolved installed mesh scene constant: " + name; return false; }
    }
    return true;
}

bool write_bitmap(const D3DLOCKED_RECT& locked, UINT width, UINT height) {
    BITMAPFILEHEADER file{}; BITMAPINFOHEADER info{};
    file.bfType = 0x4d42;
    file.bfOffBits = sizeof(file) + sizeof(info);
    file.bfSize = file.bfOffBits + width * height * 4;
    info.biSize = sizeof(info); info.biWidth = static_cast<LONG>(width);
    info.biHeight = -static_cast<LONG>(height); info.biPlanes = 1;
    info.biBitCount = 32; info.biCompression = BI_RGB;
    std::ofstream output("local/installed_mesh/render.bmp", std::ios::binary);
    output.write(reinterpret_cast<const char*>(&file), sizeof(file));
    output.write(reinterpret_cast<const char*>(&info), sizeof(info));
    for (UINT row = 0; row < height; ++row)
        output.write(static_cast<const char*>(locked.pBits) + row * locked.Pitch, width * 4);
    return static_cast<bool>(output);
}

// Actual owned state for this controlled diagnostic scene. The installed
// hierarchy is identity (checked by the reader), so its serialized sphere
// center is also its world center. Native scene graph/model wrappers remain
// separate; these callbacks retain and update this explicit host scene state.
struct MeshInstanceModel {
    bsp::CameraMatrix world{};
    std::array<float, 3> sphere_center{};
    const void* scene{};
    unsigned attachments{};
};
bool attach_mesh_scene(void* opaque, const void* scene, bool recurse, std::string& error) {
    if (!opaque || recurse) { error = "Installed mesh scene binding requires a nonrecursive model"; return false; }
    auto& model = *static_cast<MeshInstanceModel*>(opaque);
    model.scene = scene; ++model.attachments; return true;
}
bool mesh_world_sphere(void* opaque, std::array<float, 3>& center, std::string& error) {
    if (!opaque) { error = "Installed mesh model context is missing"; return false; }
    center = static_cast<MeshInstanceModel*>(opaque)->sphere_center; return true;
}
struct InstanceWriteEvidence {
    std::vector<bsp::BuildingInstanceData> records;
    bool mapped_bytes_match{true};
};
bool write_mesh_instance(void* opaque, const bsp::InstanceRenderEntry& entry,
    std::uint8_t* output, std::size_t available, std::string& error) {
    if (!entry.model || !entry.model->context || !entry.section
        || !entry.section->material_clone_owner || available < sizeof(bsp::BuildingInstanceData)) return false;
    const auto& model = *static_cast<const MeshInstanceModel*>(entry.model->context);
    const auto material = std::static_pointer_cast<bsp::MaterialCloneState>(entry.section->material_clone_owner);
    bsp::BuildingInstanceData record;
    if (!bsp::write_building_instance_data_00b55780(model.world, {}, entry.visibility,
        material->lighting.diffuse_color_00b179f0(0)[3], record, error)) return false;
    std::memcpy(output, record.data(), sizeof(record));
    auto& evidence = *static_cast<InstanceWriteEvidence*>(opaque);
    evidence.mapped_bytes_match = evidence.mapped_bytes_match
        && std::memcmp(output, record.data(), sizeof(record)) == 0;
    evidence.records.push_back(record); return true;
}
class MeshInstanceFactory final : public bsp::InstanceGroupFactory {
public:
    MeshInstanceFactory(bsp::D3D9StateCache& states,
        std::shared_ptr<bsp::VertexBufferBinding> physical,
        bsp::GeneratedInstanceGeometrySource source,
        bsp::GeneratedInstanceGeometryGenerator generator)
        : states_(states), physical_(std::move(physical)), source_(std::move(source)),
          generator_(std::move(generator)) {}
    bool create(const bsp::InstanceRenderEntry& entry, const bsp::InstanceGroupingBinding&,
        std::uint32_t, const std::array<float, 4>& tint, bsp::InstanceUploadModel& model,
        std::shared_ptr<bsp::GeneratedInstanceGeometry>& geometry, std::string& error) override {
        const auto source_material = std::static_pointer_cast<bsp::MaterialCloneState>(entry.section->material_clone_owner);
        std::shared_ptr<bsp::MaterialCloneState> material;
        auto hr = bsp::clone_material_00b18b60(*source_material, material);
        if (SUCCEEDED(hr)) hr = bsp::create_generated_instance_geometry_00b4c8d0_fragment(
            states_, physical_, source_, generator_, {material, entry.section->material_order,
                entry.section->material_queue_index}, geometry);
        if (FAILED(hr)) { error = "Generated mesh/material creation failed"; return false; }
        std::memcpy(material->lighting.diffuse_color_00b179f0(0), tint.data(), sizeof(tint));
        auto context = std::make_shared<MeshInstanceModel>(*static_cast<MeshInstanceModel*>(entry.model->context));
        model.geometry = geometry.get(); model.context = context.get(); model.context_owner = context;
        model.attach_scene = attach_mesh_scene; model.world_sphere_center = mesh_world_sphere;
        return true;
    }
private:
    bsp::D3D9StateCache& states_;
    std::shared_ptr<bsp::VertexBufferBinding> physical_;
    bsp::GeneratedInstanceGeometrySource source_;
    bsp::GeneratedInstanceGeometryGenerator generator_;
};

bool draw_mesh(IDirect3DDevice9& device, AssetStreamProbe& assets,
    const InstalledModelProbe& model, const std::shared_ptr<bsp::CompiledMaterialPass>& material_pass, std::string& error) {
    const auto& shaders = *material_pass;
    if (shaders.base.options.instance_generator != "building" || !model.hierarchy.matrix
        || shaders.shadow_samplers.shadow_texture_slot != 1 || shaders.shadow_samplers.shadow_map_slot != -1)
        return false;
    const auto& mesh = model.mesh;
    const auto& subset = mesh.subsets[0];
    bsp::MaterialLighting lighting;
    TextureImports imports;
    std::vector<std::unique_ptr<bsp::D3D9RetainedTexture2D>> textures;
    bsp::MaterialTextureSlots slots;
    std::vector<const bsp::MeshVertexStreamPayload*> ordered_streams;
    for (const auto& event : subset.events) {
        if (const auto* selected = std::get_if<bsp::MeshVertexStreamReference>(&event)) {
            if (selected->index >= mesh.vertex_streams.size()) return false;
            ordered_streams.push_back(&mesh.vertex_streams[selected->index]);
        } else if (const auto* texture = std::get_if<bsp::MeshTextureRequest>(&event)) {
            std::unique_ptr<bsp::D3D9RetainedTexture2D> owner;
            if (!load_texture(device, assets, imports, texture->name, owner, error)) return false;
            auto logical = std::make_shared<bsp::LogicalTexture>(); logical->texture = owner->texture();
            if (!bsp::set_material_texture_00b189f0(slots, texture->slot, logical)) return false;
            textures.push_back(std::move(owner));
        } else if (const auto* record = std::get_if<bsp::MeshLightingRecord>(&event))
            lighting.set_lighting_record_00b179d0(record->ignored_slot, record->values);
    }
    if (ordered_streams.size() != 1 || !mesh.indices) return false;
    // This white image is a controlled unoccluded ShadowTexture scene input.
    // Native shadow generation/global ownership is a separate remaining task.
    std::unique_ptr<bsp::D3D9RetainedTexture2D> shadow_image;
    if (!load_texture(device, assets, imports, "white.tga", shadow_image, error)) return false;
    auto shadow = std::make_shared<bsp::LogicalTexture>(); shadow->texture = shadow_image->texture();
    std::vector<float> vertex_words(256 * 4), pixel_words(224 * 4);
    bsp::MeshDecodeBindingStats decoded;
    if (!scene_constants(shaders.vr, vertex_words, true, error)
        || !scene_constants(shaders.pr, pixel_words, false, error)
        || !bsp::pack_mesh_vertex_decode_constants_00b428c0(ordered_streams, shaders.vb,
            {static_cast<std::uint32_t>(shaders.base.options.compressed_element_count), {}},
            0, vertex_words, decoded, error)
        || decoded.records_from_metadata != 3 || decoded.records_written != 3) return false;
    bsp::BuildingInstanceData instance;
    if (!bsp::write_building_instance_data_00b55780(*model.hierarchy.matrix, {}, 1,
        lighting.diffuse_color_00b179f0(0)[3], instance, error)) return false;
    auto declaration = std::make_shared<bsp::VertexDeclaration>();
    auto instance_declaration = std::make_shared<bsp::VertexDeclaration>();
    if (!bsp::decode_vertex_format_00b2dbd0(ordered_streams[0]->format_name, *declaration, error)
        || !bsp::decode_vertex_format_00b2dbd0(bsp::building_instance_vertex_format, *instance_declaration, error)) return false;

    OwnedCom<IDirect3DStateBlock9> saved;
    OwnedCom<IDirect3DSurface9> old_target, old_depth, target, depth, readback;
    HRESULT hr = device.CreateStateBlock(D3DSBT_ALL, &saved.p);
    if (SUCCEEDED(hr)) hr = device.GetRenderTarget(0, &old_target.p);
    if (SUCCEEDED(hr)) {
        const auto result = device.GetDepthStencilSurface(&old_depth.p);
        if (FAILED(result) && result != D3DERR_NOTFOUND) hr = result;
    }
    if (FAILED(hr)) return false;
    unsigned visible = 0; std::set<DWORD> colors;
    bool buffers_match = false, constants_match = false, layout_match = false, bindings_match = false;
    bool groups_match = false, capacity_guard = false;
    unsigned queued_bindings_checked = 0;
    try {
        bsp::RendererSynchronization sync;
        bsp::D3D9StateCache states(device, sync, nullptr);
        std::shared_ptr<bsp::LogicalVertexStream> vertices;
        std::shared_ptr<bsp::LogicalIndexStream> indices;
        hr = bsp::create_mesh_vertex_stream_00b4bc00_fragment(device, states,
            *ordered_streams[0], declaration, vertices);
        if (SUCCEEDED(hr)) hr = bsp::create_mesh_index_stream_00b4bf30_fragment(device, states, *mesh.indices, indices);
        auto layout = std::make_shared<bsp::D3D9VertexLayout>();
        layout->append_stream_00b48a00(declaration); layout->append_stream_00b48a00(instance_declaration);
        if (SUCCEEDED(hr)) hr = layout->create_if_missing_00b60a10(device);
        auto shared_vertices = std::make_shared<bsp::VertexBufferBinding>();
        bsp::D3D9DynamicBuffers dynamic_buffers;
        if (SUCCEEDED(hr)) hr = bsp::create_dynamic_buffers_00b2aeb0(device, dynamic_buffers);
        shared_vertices->buffer = dynamic_buffers.vertices; dynamic_buffers.vertices = nullptr;
        bsp::release_dynamic_buffers(dynamic_buffers);
        shared_vertices->flags = bsp::generated_instance_stream_flags;
        shared_vertices->capacity = bsp::generated_instance_shared_vertex_capacity;

        auto source_material = std::make_shared<bsp::MaterialCloneState>();
        source_material->textures = slots; source_material->lighting = lighting;
        source_material->effect = material_pass;
        // Ordinary constructor00b18900 initializes both offset-named words
        // toFFFFFFFF; clone00b18b60 then copies them without interpretation.
        source_material->word104 = source_material->word108 = 0xffffffffu;
        bsp::GeneratedInstanceGeometry source_geometry;
        source_geometry.mesh_stream = vertices; source_geometry.indices = indices;
        source_geometry.section.primitive = subset.native_primitive;
        source_geometry.section.range_words = subset.range_words;
        source_geometry.section.material_order = shaders.base.options.priority;
        source_geometry.section.material_queue_index = static_cast<std::uint32_t>(shaders.base.options.pipe_id);
        source_geometry.section.material_clone_owner = source_material;
        auto source_model_state = std::make_shared<MeshInstanceModel>();
        source_model_state->world = *model.hierarchy.matrix;
        std::copy_n(model.hierarchy.sphere.data(), 3, source_model_state->sphere_center.begin());
        bsp::InstanceUploadModel source_model;
        source_model.geometry = &source_geometry; source_model.context = source_model_state.get();
        source_model.context_owner = source_model_state; source_model.attach_scene = attach_mesh_scene;
        source_model.world_sphere_center = mesh_world_sphere;
        bsp::CameraTransform camera;
        bsp::set_transform_local_matrix_00b6db10(camera, *model.hierarchy.matrix);
        std::array<bsp::InstanceRenderEntry, 2> source_entries;
        for (std::uint32_t i=0; i<2 && SUCCEEDED(hr); ++i)
            if (!bsp::initialize_instance_render_entry_00b51a20(source_entries[i],0,
                source_geometry.section,source_geometry,source_model,camera,i ? .5f : 1.0f,0,0x555,error)) hr=E_FAIL;
        InstanceWriteEvidence writes;
        auto generator = std::make_shared<bsp::InstanceUploadGenerator>();
        generator->declaration = instance_declaration.get(); generator->context = &writes;
        generator->write_record = write_mesh_instance;
        auto binding = std::make_shared<bsp::InstanceGroupingBinding>();
        binding->id = 0; binding->generator = generator;
        bsp::InstanceGroupingState groups;
        bsp::InstanceRenderQueue opaque_queue, faded_queue;
        const std::vector<bsp::InstanceRenderQueue*> queues{&opaque_queue,&faded_queue};
        MeshInstanceFactory factory(states,shared_vertices,{vertices,indices,subset.native_primitive,subset.range_words},
            {instance_declaration,layout});
        bsp::InstanceVisibilityInputs visibility;
        visibility.enabled = shaders.base.options.final_lod_fade_out;
        visibility.model_world_position = {(*model.hierarchy.matrix)[12],(*model.hierarchy.matrix)[13],(*model.hierarchy.matrix)[14]};
        visibility.camera_world_position = visibility.model_world_position;
        visibility.camera_mode = 3; // Explicit host camera mode bypasses distance fade.
        visibility.descriptor_fraction = shaders.base.options.final_lod_fade_out_range;
        for (auto& entry : source_entries) {
            bsp::InstanceGroupingResult result{};
            if (SUCCEEDED(hr) && (!bsp::group_instance_render_entry_00b1dff0_fragment(
                groups,entry,visibility,binding,factory,queues,result,error)
                || result != bsp::InstanceGroupingResult::grouped)) hr=E_FAIL;
        }
        bsp::InstanceUploadStats upload;
        const unsigned scene_identity = 1;
        bsp::InstanceUploadContext upload_context{groups.ordered_groups,&scene_identity,&camera,queues};
        if (SUCCEEDED(hr)) hr = bsp::upload_instance_groups_00b1e990_fragment(states,upload_context,upload,error);
        if (FAILED(hr)) throw std::runtime_error("Instance grouping/upload: " + error);
        auto& full = groups.by_binding[0]->upload.categories[0];
        auto& faded = groups.by_binding[0]->upload.categories[1];
        const auto generated = groups.by_binding[0]->geometries[0];
        auto instances = generated->instance_stream;
        const auto faded_instances = groups.by_binding[0]->geometries[1]->instance_stream;
        const auto clone = std::static_pointer_cast<bsp::MaterialCloneState>(generated->section.material_clone_owner);
        groups_match = upload.groups_visited == 1 && upload.categories_uploaded == 2
            && upload.records_written == 2 && upload.entries_queued == 2
            && opaque_queue.entries().size() == 1 && faded_queue.entries().size() == 1
            && opaque_queue.entries()[0] == full.output_entry && faded_queue.entries()[0] == faded.output_entry
            && full.source_entries[0] == &source_entries[0] && faded.source_entries[0] == &source_entries[1]
            && full.output_entry->visibility == 1 && faded.output_entry->visibility == .5f
            && full.output_entry->flags == 0x555 && faded.output_entry->flags == 0x555
            && full.generated_model->geometry->section.instance_count == 1
            && faded.generated_model->geometry->section.instance_count == 1
            && shared_vertices->logical_streams.size() == 2 && instances->physical == shared_vertices
            && faded_instances->physical == shared_vertices && instances->offset != faded_instances->offset
            && generated->mesh_stream == vertices && generated->indices == indices && generated->combined_layout == layout
            && clone.get() != source_material.get() && clone->effect == source_material->effect
            && clone->word104 == 0xffffffffu && clone->word108 == 0xffffffffu
            && clone->textures.textures() == source_material->textures.textures() && clone->lighting.flag_10c()
            && clone->lighting.diffuse_color_00b179f0(0)[0] == 1 && clone->lighting.diffuse_color_00b179f0(0)[1] == 0
            && writes.records.size() == 2 && writes.mapped_bytes_match
            && std::memcmp(writes.records[0].data(),instance.data(),sizeof(instance)) == 0
            && writes.records[1][27] == .5f
            && static_cast<MeshInstanceModel*>(full.generated_model->context)->scene == &scene_identity
            && static_cast<MeshInstanceModel*>(faded.generated_model->context)->scene == &scene_identity;
        {
            struct RestoreCursor { bsp::VertexBufferBinding& buffer; UINT cursor;
                ~RestoreCursor() { buffer.cursor = cursor; } } restore{*shared_vertices,shared_vertices->cursor};
            const auto prior_depth = shared_vertices->lock_depth, prior_locks = shared_vertices->dynamic_locks;
            const auto prior_attachments = static_cast<MeshInstanceModel*>(full.generated_model->context)->attachments;
            shared_vertices->cursor = shared_vertices->capacity;
            bsp::InstanceUploadStats rejected_upload; std::string capacity_error;
            const auto rejected = bsp::upload_instance_groups_00b1e990_fragment(states,upload_context,rejected_upload,capacity_error);
            capacity_guard = rejected == D3DERR_INVALIDCALL && !capacity_error.empty()
                && rejected_upload.categories_uploaded == 0 && rejected_upload.records_written == 0
                && rejected_upload.entries_queued == 0 && shared_vertices->lock_depth == prior_depth
                && shared_vertices->dynamic_locks == prior_locks && shared_vertices->cursor == shared_vertices->capacity
                && static_cast<MeshInstanceModel*>(full.generated_model->context)->attachments == prior_attachments
                && opaque_queue.entries().size() == 1 && faded_queue.entries().size() == 1;
        }
        groups_match = groups_match && capacity_guard;
        std::printf("Installed instance groups: groups=%zu categories=%zu records=%zu queued=%zu pool=%u offsets=%u/%u shared_registry=%zu clone_distinct=%d capacity_guard=%d checked=%d\n",
            upload.groups_visited,upload.categories_uploaded,upload.records_written,upload.entries_queued,
            shared_vertices->capacity,instances->offset,faded_instances->offset,shared_vertices->logical_streams.size(),
            clone.get()!=source_material.get(),capacity_guard,groups_match);
        void* mapped = nullptr;
        const auto elements = layout->elements();
        layout_match = elements.size() == 13 && layout->stride() == 160;
        for (UINT i = 0; layout_match && i < 9; ++i)
            layout_match = elements[i+3].Stream == 1 && elements[i+3].Offset == i*16
                && elements[i+3].Type == D3DDECLTYPE_FLOAT4 && elements[i+3].Usage == D3DDECLUSAGE_TEXCOORD
                && elements[i+3].UsageIndex == i+1;
        if (SUCCEEDED(hr)) {
            D3DVERTEXBUFFER_DESC vd{}; D3DINDEXBUFFER_DESC id{};
            hr = vertices->physical->buffer->GetDesc(&vd);
            if (SUCCEEDED(hr)) hr = indices->physical->buffer->GetDesc(&id);
            buffers_match = SUCCEEDED(hr) && vd.Size == 192 && vd.Usage == 0 && vd.Pool == D3DPOOL_MANAGED
                && id.Size == 48 && id.Usage == 0 && id.Pool == D3DPOOL_MANAGED && id.Format == D3DFMT_INDEX16;
            if (SUCCEEDED(hr)) hr = vertices->physical->buffer->Lock(0, 0, &mapped, D3DLOCK_READONLY);
            if (SUCCEEDED(hr)) {
                buffers_match = buffers_match && std::memcmp(mapped, ordered_streams[0]->bytes.data(), vd.Size) == 0;
                hr = vertices->physical->buffer->Unlock();
            }
            if (SUCCEEDED(hr)) hr = indices->physical->buffer->Lock(0, 0, &mapped, D3DLOCK_READONLY);
            if (SUCCEEDED(hr)) {
                buffers_match = buffers_match && std::memcmp(mapped, mesh.indices->bytes.data(), id.Size) == 0;
                hr = indices->physical->buffer->Unlock();
            }
            D3DVERTEXBUFFER_DESC dynamic_description{};
            if (SUCCEEDED(hr)) hr = shared_vertices->buffer->GetDesc(&dynamic_description);
            buffers_match = buffers_match && SUCCEEDED(hr) && dynamic_description.Size == 0x1000000
                && dynamic_description.Pool == D3DPOOL_DEFAULT && dynamic_description.Usage == 0x208;
        }
        if (SUCCEEDED(hr)) hr = device.CreateRenderTarget(256,256,D3DFMT_A8R8G8B8,D3DMULTISAMPLE_NONE,0,FALSE,&target.p,nullptr);
        if (SUCCEEDED(hr)) hr = device.CreateDepthStencilSurface(256,256,D3DFMT_D24S8,D3DMULTISAMPLE_NONE,0,TRUE,&depth.p,nullptr);
        if (SUCCEEDED(hr)) hr = device.CreateOffscreenPlainSurface(256,256,D3DFMT_A8R8G8B8,D3DPOOL_SYSTEMMEM,&readback.p,nullptr);
        if (SUCCEEDED(hr)) hr = device.SetDepthStencilSurface(nullptr);
        if (SUCCEEDED(hr)) hr = device.SetRenderTarget(0,target.p);
        if (SUCCEEDED(hr)) hr = device.SetDepthStencilSurface(depth.p);
        const D3DVIEWPORT9 viewport{0,0,256,256,0,1};
        if (SUCCEEDED(hr)) hr = device.SetViewport(&viewport);
        for (const auto& setting : {std::pair<D3DRENDERSTATETYPE,DWORD>{D3DRS_CULLMODE,D3DCULL_NONE},
            {D3DRS_ZENABLE,TRUE},{D3DRS_ZWRITEENABLE,TRUE},{D3DRS_ZFUNC,D3DCMP_LESSEQUAL},
            {D3DRS_ALPHATESTENABLE,FALSE},{D3DRS_ALPHABLENDENABLE,FALSE},{D3DRS_SCISSORTESTENABLE,FALSE},
            {D3DRS_SRGBWRITEENABLE,FALSE},{D3DRS_COLORWRITEENABLE,15},{D3DRS_CLIPPLANEENABLE,0}})
            if (SUCCEEDED(hr)) hr = device.SetRenderState(setting.first,setting.second);
        for (UINT slot = 0; slot < 2; ++slot)
            if (SUCCEEDED(hr)) hr = device.SetSamplerState(slot,D3DSAMP_SRGBTEXTURE,FALSE);
        bsp::LogicalVertexShader vertex_shader{shaders.vertex}; bsp::LogicalPixelShader pixel_shader{shaders.pixel};
        if (SUCCEEDED(hr)) {
            states.bind_render_state_block_00b27a80(shaders.states);
            states.bind_sampler_state_block_00b27b90(std::make_shared<bsp::SamplerStateBlock>(shaders.pass.sampler_states));
            hr = states.bind_vertex_shader_00b21d10(&vertex_shader);
        }
        if (SUCCEEDED(hr)) hr = states.bind_pixel_shader_00b21c20(&pixel_shader);
        if (SUCCEEDED(hr)) hr = states.bind_vertex_layout_00b23f20(layout);
        if (SUCCEEDED(hr)) {
            states.bind_vertex_stream_00b24840(0, vertices); states.bind_vertex_stream_00b24840(1, instances);
            states.set_stream_frequency_00b24a40(0, vertices->tag | 1);
            states.set_stream_frequency_00b24a40(1, instances->tag | 1);
            states.bind_index_stream_00b24b00(indices, static_cast<INT>(vertices->base_vertex));
            hr = bsp::bind_material_textures_00b43470(states, shaders.pass, slots.textures(), shaders.pb.sampler_mask);
        }
        if (SUCCEEDED(hr)) hr = bsp::bind_material_shadow_samplers_00b430cf_fragment(states, shaders.shadow_samplers, {shadow,false,false,{}});
        if (SUCCEEDED(hr)) hr = states.set_vertex_shader_constants_f_00b21820(0,vertex_words.data(),shaders.vb.end_register);
        if (SUCCEEDED(hr)) hr = states.set_pixel_shader_constants_f_00b218c0(0,pixel_words.data(),shaders.pb.end_register);
        if (SUCCEEDED(hr)) {
            std::array<float,24> values{};
            hr = device.GetVertexShaderConstantF(shaders.vb.registers[24],values.data(),6);
            constants_match = SUCCEEDED(hr) && std::memcmp(values.data(),
                vertex_words.data()+shaders.vb.registers[24]*4,sizeof(values)) == 0;
            UINT f0=0,f1=0; device.GetStreamSourceFreq(0,&f0); device.GetStreamSourceFreq(1,&f1);
            OwnedCom<IDirect3DBaseTexture9> t0,t1;
            device.GetTexture(0,&t0.p); device.GetTexture(1,&t1.p);
            bindings_match = f0 == 0x40000001 && f1 == 0x80000001
                && t0.p == slots.textures()[0]->texture && t1.p == shadow->texture;
        }
        if (SUCCEEDED(hr) && !(buffers_match && constants_match && layout_match && bindings_match && groups_match)) hr = E_FAIL;
        if (SUCCEEDED(hr)) hr = device.Clear(0,nullptr,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,0xff000000,1,0);
        if (SUCCEEDED(hr)) hr = device.BeginScene();
        if (SUCCEEDED(hr)) {
            for (const auto* queue : queues) for (const auto* entry : queue->entries()) {
                if (FAILED(hr)) continue;
                const auto& geometry = *entry->geometry;
                const auto& section = *entry->section;
                hr = states.bind_vertex_layout_00b23f20(geometry.combined_layout);
                states.bind_index_stream_00b24b00(geometry.indices,static_cast<INT>(geometry.mesh_stream->base_vertex));
                states.bind_vertex_stream_00b24840(0,geometry.mesh_stream);
                states.bind_vertex_stream_00b24840(1,geometry.instance_stream);
                states.set_stream_frequency_00b24a40(0,geometry.mesh_stream->tag | section.instance_count);
                states.set_stream_frequency_00b24a40(1,geometry.instance_stream->tag | 1);
                OwnedCom<IDirect3DVertexBuffer9> actual_mesh, actual_instances;
                OwnedCom<IDirect3DIndexBuffer9> actual_indices;
                OwnedCom<IDirect3DVertexDeclaration9> actual_layout;
                UINT mesh_offset=0, mesh_stride=0, instance_offset=0, instance_stride=0, mesh_frequency=0, instance_frequency=0;
                if (SUCCEEDED(hr)) hr = device.GetStreamSource(0,&actual_mesh.p,&mesh_offset,&mesh_stride);
                if (SUCCEEDED(hr)) hr = device.GetStreamSource(1,&actual_instances.p,&instance_offset,&instance_stride);
                if (SUCCEEDED(hr)) hr = device.GetStreamSourceFreq(0,&mesh_frequency);
                if (SUCCEEDED(hr)) hr = device.GetStreamSourceFreq(1,&instance_frequency);
                if (SUCCEEDED(hr)) hr = device.GetIndices(&actual_indices.p);
                if (SUCCEEDED(hr)) hr = device.GetVertexDeclaration(&actual_layout.p);
                const bool queue_binding = SUCCEEDED(hr)
                    && actual_mesh.p == geometry.mesh_stream->physical->buffer
                    && actual_instances.p == geometry.instance_stream->physical->buffer
                    && actual_indices.p == geometry.indices->physical->buffer
                    && actual_layout.p == geometry.combined_layout->native()
                    && mesh_offset == geometry.mesh_stream->offset && mesh_stride == geometry.mesh_stream->declaration->stride
                    && instance_offset == geometry.instance_stream->offset && instance_stride == geometry.instance_stream->declaration->stride
                    && mesh_frequency == (geometry.mesh_stream->tag | section.instance_count)
                    && instance_frequency == (geometry.instance_stream->tag | 1);
                bindings_match = bindings_match && queue_binding;
                if (!queue_binding) hr = E_FAIL;
                else ++queued_bindings_checked;
                if (SUCCEEDED(hr)) hr = states.draw_indexed_00b24010({},static_cast<D3DPRIMITIVETYPE>(section.primitive),
                    section.range_words[0],section.range_words[1],section.range_words[2]+geometry.indices->base_index,section.range_words[3]);
            }
            const auto ended = device.EndScene(); if (SUCCEEDED(hr)) hr = ended;
        }
        if (SUCCEEDED(hr)) hr = device.GetRenderTargetData(target.p,readback.p);
        D3DLOCKED_RECT pixels{};
        if (SUCCEEDED(hr)) hr = readback.p->LockRect(&pixels,nullptr,D3DLOCK_READONLY);
        if (SUCCEEDED(hr)) {
            struct UnlockSurface {
                IDirect3DSurface9* surface;
                ~UnlockSurface() { if (surface) surface->UnlockRect(); }
            } unlock{readback.p};
            for (UINT y=0;y<256;++y) for (UINT x=0;x<256;++x) {
                DWORD color; std::memcpy(&color,static_cast<const char*>(pixels.pBits)+y*pixels.Pitch+x*4,4);
                if (color & 0xffffff) { ++visible; colors.insert(color); }
            }
            if (!write_bitmap(pixels,256,256)) hr = E_FAIL;
            const auto unlocked = readback.p->UnlockRect(); unlock.surface = nullptr;
            if (SUCCEEDED(hr)) hr = unlocked;
        }
        states.bind_vertex_shader_00b21d10(nullptr); states.bind_pixel_shader_00b21c20(nullptr);
        states.bind_texture_00b24710(0,{}); states.bind_texture_00b24710(1,{});
        states.bind_vertex_stream_00b24840(0,{}); states.bind_vertex_stream_00b24840(1,{});
        states.bind_index_stream_00b24b00({},0); states.bind_vertex_layout_00b23f20({});
        states.set_stream_frequency_00b24a40(0,1); states.set_stream_frequency_00b24a40(1,1);
        device.SetVertexDeclaration(nullptr); states.invalidate();
    } catch (const std::exception& exception) { error = exception.what(); hr = E_FAIL; }
    bool restored = SUCCEEDED(device.SetDepthStencilSurface(nullptr));
    restored = SUCCEEDED(device.SetRenderTarget(0,old_target.p)) && restored;
    restored = SUCCEEDED(device.SetDepthStencilSurface(old_depth.p)) && restored;
    restored = SUCCEEDED(saved.p->Apply()) && restored;
    const bool checked = SUCCEEDED(hr) && buffers_match && constants_match && layout_match && bindings_match && groups_match
        && queued_bindings_checked == 2 && visible > 32 && visible < 16384 && colors.size() > 10 && restored;
    std::printf("Installed mesh draw: hr=0x%08lx vertices=%u primitives=%u GPU_bytes=%d decode_records=%zu constant_readback=%d instance_layout=%d textures_frequencies=%d queued_bindings=%u visible=%u colors=%zu restored=%d checked=%d error=%s\n",
        static_cast<unsigned long>(hr),ordered_streams[0]->count,subset.range_words[3],buffers_match,
        decoded.records_from_metadata,constants_match,layout_match,bindings_match,queued_bindings_checked,visible,colors.size(),restored,checked,error.c_str());
    return checked;
}
}

bool probe_installed_mesh(IDirect3DDevice9& device, AssetStreamProbe& assets) {
    InstalledModelProbe model;
    if (!probe_model_metadata(assets, &model)) return false;
    std::string descriptor, error;
    if (!bsp::shader_descriptor_name_00b2ebb0_fragment(model.mesh.subsets[0].effect_name, descriptor)) return false;
    const bsp::ShaderScriptResolver resolver = [&](const std::string& requested, std::string& bytes, std::string& message) {
        std::shared_ptr<bsp::MemoryStream> stream;
        std::string logical;
        if (!assets.read(requested, stream, message, &logical)) return false;
        std::printf("Mounted mesh shader lookup: %s -> %s\n", requested.c_str(), logical.c_str());
        bytes.assign(reinterpret_cast<const char*>(stream->data_00bef610()),
            static_cast<std::size_t>(stream->size_00bef600()));
        return true;
    };
    std::shared_ptr<bsp::CompiledMaterialPass> shaders;
    if (!bsp::compile_material_pass(device, resolver, descriptor, {0,3,false,false}, shaders, error)) {
        std::fprintf(stderr, "Installed mesh compiler: %s\n", error.c_str()); return false;
    }
    std::filesystem::create_directories("local/installed_mesh");
    std::ofstream("local/installed_mesh/vertex.hlsl", std::ios::binary) << shaders->vertex_source;
    std::ofstream("local/installed_mesh/pixel.hlsl", std::ios::binary) << shaders->pixel_source;
    for (const auto& stage : {std::pair<const char*, const std::vector<bsp::ReflectedShaderConstant>*>{"VS", &shaders->vr}, {"PS", &shaders->pr}})
        for (const auto& constant : *stage.second)
            std::printf("Installed mesh %s constant %s: set=%u type=%u index=%u count=%u rows=%u columns=%u\n",
                stage.first, constant.name.c_str(), constant.register_set, constant.parameter_type,
                constant.register_index, constant.register_count, constant.rows, constant.columns);
    std::printf("Installed mesh compiled: descriptor=%s combiner=%s samplers=%u/%u usage=%u instance=%s\n",
        descriptor.c_str(), shaders->base.combiners[0].c_str(), shaders->sampler_counts.pixel,
        shaders->sampler_counts.vertex, shaders->pb.sampler_mask, shaders->base.options.instance_generator.c_str());
    return draw_mesh(device, assets, model, shaders, error);
}
