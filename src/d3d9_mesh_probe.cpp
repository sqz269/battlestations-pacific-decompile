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
#include "bsp/material_textures.hpp"
#include "bsp/d3d9_texture.hpp"
#include "bsp/texture_load_policy.hpp"
#include "bsp/instance_grouping.hpp"
#include "bsp/instance_geometry.hpp"
#include "bsp/material_clone.hpp"
#include "bsp/d3d9_resources.hpp"
#include "bsp/native_renderer_parameters.hpp"
#include "bsp/model_bounds.hpp"
#include "bsp/scene_attachment.hpp"
#include "bsp/generated_model_lifetime.hpp"
#include "bsp/camera_frame_state.hpp"
#include "bsp/camera_multiply.hpp"
#include "bsp/material_constants.hpp"
#include "bsp/material_constant_builder.hpp"
#include "bsp/material_entry_dispatch.hpp"
#include "bsp/system_constant_builder.hpp"
#include "bsp/system_camera_axes.hpp"
#include "bsp/system_time_constants.hpp"
#include "bsp/system_lighting_constants.hpp"
#include "bsp/environment_fog_apply.hpp"
#include "bsp/system_fog_world_factory.hpp"
#include "bsp/legacy_crt_math.hpp"
#include "bsp/particle_clock_lifetime.hpp"
#include "bsp/directional_light_owner.hpp"
#include "bsp/light_type_bootstrap.hpp"
#include "bsp/lighting_configuration_apply.hpp"
#include <algorithm>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <set>
#include <stdexcept>
#include <cstdio>
#include <fstream>
#include <filesystem>
#include <unordered_map>

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
    std::unique_ptr<bsp::D3D9RetainedTexture2D>& output,
    bsp::MaterialSortMetadataCounters& counters, std::string& error) {
    std::shared_ptr<bsp::MemoryStream> source;
    auto name = requested;
    bsp::lowercase_resource_name_004bcc00(name);
    std::string logical;
    if (!imports.info || !imports.create || !assets.read(name, source, error, &logical)) return false;
    const auto result = bsp::load_retained_texture_2d_00b2c2d0_fragment(device,
        imports.info, imports.create, source,
        {static_cast<std::uint32_t>(logical.size()), logical.c_str()}, 0, output, &counters);
    std::printf("Mounted mesh texture: %s -> %s hr=0x%08lx\n", requested.c_str(), logical.c_str(),
        static_cast<unsigned long>(result));
    return SUCCEEDED(result) && output && output->texture();
}

// A single actual construction domain for this isolated mesh scene. Lowercase
// name reuse retains the same COM/logical owner and serial, including error.tga.
// This is controlled diagnostic ownership, not native texture-cache reconstruction.
class MeshTextureDomain {
public:
    MeshTextureDomain(IDirect3DDevice9& device, AssetStreamProbe& assets)
        : device_(device), assets_(assets) {}
    bsp::MaterialSortMetadataCounters counters{0, 0};
    std::shared_ptr<bsp::LogicalTexture> acquire(std::string name, std::string& error) {
        bsp::lowercase_resource_name_004bcc00(name);
        const auto found = images_.find(name);
        if (found != images_.end()) return found->second.logical;
        Image image;
        if (!load_texture(device_, assets_, imports_, name, image.owner, counters, error)) return {};
        image.logical = std::make_shared<bsp::LogicalTexture>();
        image.logical->texture = image.owner->texture();
        image.logical->sort_metadata = image.owner->sort_metadata();
        const auto logical = image.logical;
        images_.emplace(std::move(name), std::move(image));
        return logical;
    }
private:
    struct Image {
        std::unique_ptr<bsp::D3D9RetainedTexture2D> owner;
        std::shared_ptr<bsp::LogicalTexture> logical;
    };
    IDirect3DDevice9& device_;
    AssetStreamProbe& assets_;
    TextureImports imports_;
    std::unordered_map<std::string, Image> images_;
};

struct MeshParticleLifetimeCallbacks {
    bsp::ConcreteParticleClockLifetimeAccess* access{};
    unsigned& destroyed;
    bsp::TypeIdCounterLifetime* types{};
    bsp::TypeIdCounterStorage* volatile* type_slot{};
    unsigned types_destroyed{};
    static void destroy(void* context, void* owner, std::uint32_t flags) noexcept {
        auto& self = *static_cast<MeshParticleLifetimeCallbacks*>(context);
        if (self.type_slot && owner == *self.type_slot) {
            self.types->deleting_destructor_006fad40(
                static_cast<bsp::TypeIdCounterStorage*>(owner), flags);
            ++self.types_destroyed;
            return;
        }
        self.access->deleting_destructor_004de340(static_cast<bsp::ParticleClock*>(owner),flags);
        ++self.destroyed;
    }
    static void invalid_parameter(void*) { _invalid_parameter_noinfo(); }
};
struct MeshSingletonShutdown {
    bsp::SingletonLifetimeDomain& domain;
    ~MeshSingletonShutdown() { domain.shutdown(); }
};
struct MeshFogCameraShutdown {
    bsp::SystemFogSlotRef slot;
    ~MeshFogCameraShutdown() { bsp::clear_system_fog_camera_slot_00b71f68(slot); }
};

struct MeshLightTypes {
    // Explicit zero-filled process-start preimage for this isolated type domain.
    std::uint8_t root_guard{}, node_guard{}, light_guard{}, directional_guard{};
    bsp::RootTypeDescriptor root{};
    bsp::NodeTypeDescriptor node{};
    bsp::LightTypeDescriptor light{};
    bsp::DirectionalLightTypeDescriptor directional{};
    bsp::LightTypeBootstrap bootstrap;
    explicit MeshLightTypes(bsp::TypeIdCounterLifetime& counter)
        : bootstrap(counter, {root_guard, root, node_guard, node, light_guard, light,
            directional_guard, directional}) {
        bootstrap.initialize_directional_00cd80f0();
    }
};
struct MeshLightSceneRuntime final : bsp::SceneAttachmentRuntime {
    bsp::LightTypeBootstrap& types;
    explicit MeshLightSceneRuntime(MeshLightTypes& value)
        // No c3dObject owner/dispatch is supplied or invoked in this light scope.
        : SceneAttachmentRuntime(value.light.own_id, {}), types(value.bootstrap) {}
    static bool node_type(bsp::SceneAttachmentRuntime& runtime,
        bsp::SceneNodeAttachment&, std::uint32_t token) {
        return static_cast<MeshLightSceneRuntime&>(runtime).types.node_is_type_00b6f570(token);
    }
    static bool light_type(bsp::SceneAttachmentRuntime& runtime,
        bsp::SceneNodeAttachment&, std::uint32_t token) {
        return static_cast<MeshLightSceneRuntime&>(runtime).types.light_is_type_00b7c580(token);
    }
    static bool directional_type(bsp::SceneAttachmentRuntime& runtime,
        bsp::SceneNodeAttachment&, std::uint32_t token) {
        return static_cast<MeshLightSceneRuntime&>(runtime).types.directional_is_type_00b7c6d0(token);
    }
};
struct MeshAbsentShadow final : bsp::SystemShadowOwnerResolver {
    bsp::SystemShadowMapOwner* resolve_shadow(void*) override {
        throw std::logic_error("Installed light has no constructed native shadow owner");
    }
};
struct MeshLightPoolShutdown {
    bsp::DirectionalLightPool& pool;
    ~MeshLightPoolShutdown() { pool.destroy_00b7b230(); }
};
struct MeshLightSlotShutdown {
    bsp::DirectionalLightPool& pool;
    void* raw;
    ~MeshLightSlotShutdown() { if (raw) pool.return_raw_slot_00b7b2f0(raw); }
};
struct MeshLightShutdown {
    bsp::DirectionalLightOwner* owner;
    void close() {
        if (owner) {
            auto* captured = owner;
            owner = nullptr;
            bsp::delete_native_directional_light_00b7c820(*captured, 1);
        }
    }
    ~MeshLightShutdown() { close(); }
};
struct MeshLightingSceneRelease {
    void operator()(bsp::ConcreteSystemSceneResource* value) const noexcept {
        if (value && value->references.fetch_sub(1) == 1) value->destroy_on_zero(*value);
    }
};
struct MeshLightingSlotShutdown {
    bsp::SceneResource*& slot;
    ~MeshLightingSlotShutdown() { bsp::set_system_scene_resource_00b723f0(slot, nullptr); }
};

bool gpu_system_prefix_matches(IDirect3DDevice9& device,
    const bsp::SystemConstantPrefix& prefix) {
    bsp::SystemConstantPrefix vertex{}, pixel{};
    return SUCCEEDED(device.GetVertexShaderConstantF(0,vertex.data(),77))
        && SUCCEEDED(device.GetPixelShaderConstantF(0,pixel.data(),77))
        && std::memcmp(vertex.data(),prefix.data(),sizeof(prefix)) == 0
        && std::memcmp(pixel.data(),prefix.data(),sizeof(prefix)) == 0;
}

bool build_mesh_system_constants(bsp::CameraFrameState& camera,
    bsp::D3D9StateCache* const volatile& renderer,
    bsp::ParticleClock* volatile& particle_slot, bsp::ParticleClockLifetimeAccess& lifetime,
    bsp::TypeIdCounterLifetime& type_counter, bsp::SizedStoragePool& strings,
    bool& light_ownership_checked, bsp::SystemConstantPrefix& prefix,
    HRESULT& result, std::string& error) {
    // Explicit isolated-scene owners. Native factories/world initialization
    // are not implied by these chosen values or by the zero scratch preimage.
    bsp::FrameClock frame;
    if (!bsp::initialize_frame_clock_00bedbd0(frame)
        || !bsp::update_frame_clock_00bedc30(frame)) {
        error = "Installed mesh system clock could not sample QPC"; return false;
    }
    auto* frame_slot = &frame;
    auto* particle = bsp::particle_clock_singleton_004de4b0(particle_slot, lifetime);
    if (!particle) { error = "Installed mesh particle clock allocation failed"; return false; }
    // This recovered updater establishes +18h before the shader reads it.
    bsp::set_particle_clock_time_00b19a10(*particle, 0.0f);
    bsp::FoliageGroupManager foliage_manager;
    foliage_manager.shader_time = 0.0f; // Explicit scene value for native +2Ch.
    auto* manager_slot = &foliage_manager;
    const float foliage_time = 0, render_time = 0;
    const std::uint8_t foliage_flag = 0, render_flag = 0;
    bsp::SystemFoliageTimeOwner foliage{foliage_time, foliage_flag};
    auto* foliage_slot = &foliage;
    const bsp::CameraMatrix screen_texture{.5f,0,0,0, 0,-.5f,0,0, 0,0,1,0, .5f,.5f,0,1};
    bsp::SystemRenderTimeOwner service{{render_flag, render_time}, screen_texture};
    auto* service_slot = &service;
    bsp::ConcreteSystemTimeTimerVirtuals timer_virtuals;
    bsp::SystemTimeBindings time{frame_slot, particle_slot, manager_slot,
        foliage_slot, service_slot, lifetime, timer_virtuals};

    const auto words4 = [](std::array<float,4> values) {
        bsp::SystemLightingWords4 words;
        std::memcpy(words.data(),values.data(),sizeof(words)); return words;
    };
    const auto ambient = words4({.35f,.35f,.35f,1});
    const auto diffuse = words4({.65f,.65f,.65f,1});
    const auto specular = words4({0,0,0,0});
    std::array<bsp::SystemLightingWords4,6> cube; cube.fill(ambient);
    MeshLightTypes types(type_counter);
    MeshLightSceneRuntime scenes(types);
    bsp::GeneratedModelLifetimeRuntime attachments(scenes);
    bsp::NativeNodeDestructionRuntime node_runtime(scenes, attachments, strings,
        &MeshLightSceneRuntime::node_type);
    bsp::AllocatorListElement* allocator_head = nullptr;
    bsp::AllocatorListDomain allocators(allocator_head);
    bsp::DirectionalLightPoolStorage pool_storage{};
    bsp::DirectionalLightPool pool(allocators, pool_storage);
    pool.initialize_00b7b940();
    MeshLightPoolShutdown pool_shutdown{pool};
    MeshLightSlotShutdown slot_shutdown{pool, pool.allocate_raw_slot_00b7bac0()};
    const bsp::NativeString empty_name;
    auto native = bsp::construct_native_directional_light_00b7c6b0(
        slot_shutdown.raw, bsp::DirectionalLightPool::slot_bytes, empty_name, strings);
    MeshAbsentShadow shadow;
    bsp::DirectionalLightOwner light(native, pool, node_runtime,
        &MeshLightSceneRuntime::directional_type, &MeshLightSceneRuntime::light_type, shadow);
    scenes.bind(light.node.scene_attachment);
    MeshLightShutdown light_shutdown{&light};
    slot_shutdown.raw = nullptr; // the bound deleting destructor now returns it
    std::unique_ptr<bsp::ConcreteSystemSceneResource, MeshLightingSceneRelease> resource(
        bsp::allocate_system_scene_resource_00b83c50(strings, empty_name, scenes));
    bsp::SceneResource* scene_slot = nullptr;
    bsp::set_system_scene_resource_00b723f0(scene_slot, resource.get());
    MeshLightingSlotShutdown scene_shutdown{scene_slot};
    bsp::SystemSceneResourceSlot scene(scene_slot);

    // The recovered configuration route initializes every shader-consumed field.
    // Chosen diagnostic values are not an authored map/environment record.
    const auto raw_float = [](float value) {
        std::uint32_t word; std::memcpy(&word, &value, sizeof(word)); return word;
    };
    const auto one = raw_float(1), half = raw_float(.5f);
    const auto elevation = raw_float(.6154797087f), yaw = raw_float(.7853981634f);
    const bsp::LightingConfigurationFields config{ambient, ambient, cube, diffuse, diffuse,
        specular, one, one, yaw, elevation};
    std::uint32_t half_guard{}, diffuse_guard{}, specular_guard{};
    bsp::SystemLightingWords4 half_fallback{}, diffuse_fallback{}, specular_fallback{};
    const bsp::SystemLightingWords3 unused_direction{};
    const std::uint8_t scale_ambient = 1;
    bsp::LightingConfigurationGlobals globals{half_guard, half_fallback, diffuse_guard,
        diffuse_fallback, specular_guard, specular_fallback, unused_direction, scale_ambient, half, one};
    auto& fields = light.light;
    bsp::LightingDirectionalFields directional_fields{fields.diffuse_184, fields.specular_194,
        fields.base_diffuse_1a4, fields.diffuse_mode3_1b4, fields.base_specular_1c4,
        fields.diffuse_scale_1d8, fields.specular_scale_1dc, fields.direction_1e0};
    bsp::apply_lighting_configuration_004bacd0(
        bsp::lighting_ambient_fields(*resource->environment_10), directional_fields, &config, globals);
    try {
        bsp::attach_light_scene_00b7c020(scenes, light.retained_scenes, resource.get(), false);
    } catch (...) {
        // This isolated light started with an empty back array. Native attach
        // publishes its entry before registry allocation and the final retain.
        // Give that partial entry its real reference before the diagnostic
        // guards run the native deleting destructor and release the two owners.
        // The recovered attach routine itself keeps its original ordering.
        if (fields.scenes_178.count_04 == 1 && fields.scenes_178.begin_00[0] == resource.get())
            resource->references.fetch_add(1);
        throw;
    }
    const bool light_bound = resource->registry.size() == 1
        && scenes.resolve_light(light.node.scene_attachment.pointer_key) == &light.lighting
        && &light.lighting.diffuse_184 == &fields.diffuse_184
        && &light.lighting.specular_194 == &fields.specular_194
        && light.lighting.shadow_owner_174() == nullptr;

    // Diagnostic runtime words use verified PE preimages, including zero fill.
    // Binding persists safely after this draw; actual native startup may alter it.
    static const std::uint32_t axes_bypass = 0, matherr_bypass = 0x2694;
    static const bsp::LegacyCrtMathRuntime crt{&matherr_bypass, &_errno};
    bsp::bind_legacy_crt_math_runtime(crt);
    const bsp::CameraAxesCrtAccess axes{&axes_bypass,&bsp::legacy_crt_87except_00c27489};
    const std::array<float,16> parameters{};
    const volatile std::uint32_t register_count = 77;
    const std::uint32_t exponent = 0x43000000;
    bsp::SystemConstantBuilderEnvironment bindings{time,axes,parameters.data(),
        renderer,register_count,exponent,nullptr};
    const bool built = bsp::build_and_upload_system_constants_00b46a70(&scene,camera,prefix,
        bindings,result,error);
    light_shutdown.close();
    bsp::set_system_scene_resource_00b723f0(scene_slot, nullptr);
    light_ownership_checked = light_bound && resource->registry.size() == 0
        && resource->references == 1 && resource->environment_10->references_04 == 1;
    resource.reset(); // final scene and ambient release
    pool.trim_empty_slabs_00b7ba20();
    light_ownership_checked = light_ownership_checked && pool_storage.slab_count_2c == 0
        && pool_storage.recursion_24 == 0;
    return built && light_ownership_checked;
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

struct MeshModelLifetimeEvidence {
    unsigned constructed{}, disposed{};
    bool cleanup_matches{true};
};
struct MeshInstanceModel;
struct MeshModelControl final : bsp::GeneratedModelStorageOwner {
    MeshInstanceModel* model{};
    MeshModelLifetimeEvidence& evidence;
    explicit MeshModelControl(MeshModelLifetimeEvidence& value) : evidence(value) {}
    ~MeshModelControl();
    void dispose_model_storage(bsp::GeneratedModelLifetime&) noexcept override;
};
// Shared host control owns the physical allocation. Native virtual18/terminal
// cleanup can dispose it first; the control then retains only a null pointer.
struct MeshInstanceModel {
    bsp::CameraTransform transform;
    bsp::ModelBounds bounds;
    bsp::SceneAttachmentRuntime& runtime;
    bsp::SceneNodeAttachment attachment;
    bsp::GeneratedModelLifetime lifetime;
    unsigned attachments{};
    MeshInstanceModel(bsp::GeneratedModelLifetimeRuntime& models,
        const bsp::CameraMatrix& local, const std::array<float, 4>& local_sphere,
        bsp::GeneratedModelGeometryReference& geometry, MeshModelControl& owner)
        : runtime(models.scenes), attachment(transform,
            static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(this)),
            bsp::object_accepts_scene_type_006ef860, bsp::set_node_scene_00b6ed80),
          lifetime(models, attachment, {1, 0, nullptr, &geometry, nullptr, {}, {}}, owner) {
        bsp::set_transform_local_matrix_00b6db10(transform, local);
        bounds.local_sphere = local_sphere;
        runtime.bind(attachment);
        try { models.bind(lifetime); }
        catch (...) { runtime.unbind(attachment); throw; }
    }
    MeshInstanceModel(const MeshInstanceModel&) = delete;
    MeshInstanceModel& operator=(const MeshInstanceModel&) = delete;
};
class MeshMaterialConstantOwners final : public bsp::MaterialConstantOwners {
public:
    MeshMaterialConstantOwners(bsp::CameraFrameState& camera,
        std::shared_ptr<bsp::LogicalTexture> shadow)
        : camera_(camera), shadow_(std::move(shadow)) {}
    bool material(bsp::InstanceRenderEntry& entry, bsp::MaterialCloneState*& value,
        std::string& error) override {
        if (!entry.section || !entry.section->material_clone_owner)
            return missing("actual section material", error);
        value = static_cast<bsp::MaterialCloneState*>(entry.section->material_clone_owner.get());
        return true;
    }
    bool camera_mode(bsp::InstanceRenderEntry& entry, std::uint32_t& value,
        std::string& error) override {
        if (entry.camera != &camera_.camera.transform) return missing("actual camera companion", error);
        value = camera_.render_mode; return true;
    }
    bool transform(bsp::InstanceRenderEntry& entry, bsp::CameraTransform*& value,
        std::string& error) override {
        auto* owner = model(entry, error);
        if (!owner) return false;
        value = &owner->transform; return true;
    }
    bool bone_model(bsp::InstanceRenderEntry&, bsp::MaterialBoneModel*&, std::string& error) override {
        return missing("bone-model owner (this scene has ordinary generated models)", error);
    }
    bool skin_model(bsp::InstanceRenderEntry&, bsp::MaterialSkinModel*&, std::string& error) override {
        return missing("skin-model owner (this scene has ordinary generated models)", error);
    }
    bool decode_inputs(bsp::InstanceRenderEntry& entry, const bsp::CompiledMaterialPass& pass,
        std::vector<const bsp::LogicalVertexStream*>& streams,
        bsp::MeshDecodeDescriptorLimits& limits, std::string& error) override {
        if (!entry.geometry || &entry.geometry->section != entry.section
            || !entry.geometry->mesh_stream || !entry.geometry->instance_stream)
            return missing("actual generated section streams", error);
        streams = {entry.geometry->mesh_stream.get(), entry.geometry->instance_stream.get()};
        limits.element_limit20 = static_cast<std::uint32_t>(pass.base.options.compressed_element_count);
        // This NORMAL pass has no configured ShadowShader descriptor owner.
        limits.element_limit24.reset();
        return true;
    }
    bool lod_threshold(bsp::InstanceRenderEntry&, const float*&, std::string& error) override {
        return missing("selected stream LOD parameter owner", error);
    }
    bool model_lifetime(bsp::InstanceRenderEntry& entry, bsp::GeneratedModelLifetime*& value,
        std::string& error) override {
        auto* owner = model(entry, error);
        if (!owner) return false;
        value = &owner->lifetime; return true;
    }
    bool shadow_texture(std::shared_ptr<bsp::LogicalTexture>& value, std::string&) override {
        value = shadow_; return true; // Actual explicit unoccluded host-scene image.
    }
    bool first_shadow_light(bsp::InstanceRenderEntry&, bool&, bsp::MaterialShadowMapOwner*&,
        std::string& error) override {
        return missing("selected directional-light shadow owner", error);
    }
private:
    static bool missing(const char* value, std::string& error) {
        error = std::string("Installed mesh scene has no ") + value; return false;
    }
    static MeshInstanceModel* model(bsp::InstanceRenderEntry& entry, std::string& error) {
        if (!entry.model || !entry.model->context) { missing("actual model", error); return nullptr; }
        return static_cast<MeshInstanceModel*>(entry.model->context);
    }
    bsp::CameraFrameState& camera_;
    std::shared_ptr<bsp::LogicalTexture> shadow_;
};
MeshModelControl::~MeshModelControl() {
    if (model) bsp::unlink_and_release_render_model_00b6dfa0(model->lifetime);
}
void MeshModelControl::dispose_model_storage(bsp::GeneratedModelLifetime& value) noexcept {
    auto* disposed = model;
    if (!disposed || &disposed->lifetime != &value) std::terminate();
    evidence.cleanup_matches = evidence.cleanup_matches && value.released_byte_44 == 1
        && !value.retained_174 && !value.geometry_180 && !value.retained_130
        && !disposed->attachment.scene && value.point_lights_164.empty();
    model = nullptr;
    ++evidence.disposed;
    delete disposed;
}
std::shared_ptr<MeshModelControl> create_mesh_model(bsp::GeneratedModelLifetimeRuntime& runtime,
    const bsp::CameraMatrix& local, const std::array<float, 4>& sphere,
    const std::shared_ptr<bsp::GeneratedInstanceGeometry>& geometry, MeshModelLifetimeEvidence& evidence) {
    auto control = std::make_shared<MeshModelControl>(evidence);
    auto retained = std::make_unique<bsp::GeneratedModelGeometryReference>(geometry);
    control->model = new MeshInstanceModel(runtime, local, sphere, *retained, *control);
    retained.release();
    ++evidence.constructed;
    return control;
}
struct MeshModelLifetimes final : bsp::RenderCommandModelLifetimes {
    bsp::RenderCommandModelLifetime& for_model(bsp::InstanceUploadModel& model) noexcept override {
        return static_cast<MeshInstanceModel*>(model.context)->lifetime;
    }
};
struct MeshRenderCommand {
    MeshModelLifetimes models;
    bsp::RenderCommand command{models};
    MeshRenderCommand() {
        auto first = std::make_unique<bsp::RenderCommandBatch>();
        auto second = std::make_unique<bsp::RenderCommandBatch>();
        first->preparation_mode = 0; second->preparation_mode = 1;
        command.batches = {first.release(), second.release()};
    }
    ~MeshRenderCommand() { bsp::destroy_render_command_00b1ddd0(command); }
};
struct ReleaseMeshScene {
    void operator()(bsp::SceneResource* scene) const {
        if (scene && scene->references.fetch_sub(1) == 1) scene->destroy_on_zero(*scene);
    }
};
bool attach_mesh_scene(void* opaque, const void* scene, bool recurse, std::string& error) {
    if (!opaque) { error = "Installed mesh model context is missing"; return false; }
    auto& model = *static_cast<MeshInstanceModel*>(opaque);
    bsp::set_node_scene_00b6ed80(model.runtime, model.attachment,
        const_cast<bsp::SceneResource*>(static_cast<const bsp::SceneResource*>(scene)), recurse);
    ++model.attachments; return true;
}
bool mesh_world_sphere(void* opaque, std::array<float, 3>& center, std::string& error) {
    if (!opaque) { error = "Installed mesh model context is missing"; return false; }
    auto& model = *static_cast<MeshInstanceModel*>(opaque);
    const auto& sphere = bsp::get_model_world_sphere_00b6e8c0(model.transform, model.bounds);
    std::copy_n(sphere.data(), 3, center.begin()); return true;
}
struct InstanceWriteEvidence {
    std::vector<bsp::BuildingInstanceData> records;
    bool mapped_bytes_match{true};
};
bool read_mesh_material_key(void*, const bsp::InstanceRenderEntry& entry,
    bsp::RenderBatchMaterialKeyFields& fields, std::string& error) {
    if (!entry.section || !entry.section->material_clone_owner) {
        error = "Queued mesh entry has no retained material"; return false;
    }
    const auto material = std::static_pointer_cast<bsp::MaterialCloneState>(entry.section->material_clone_owner);
    if (!material->effect) { error = "Queued mesh material has no retained effect"; return false; }
    const auto pass = std::static_pointer_cast<const bsp::CompiledMaterialPass>(material->effect);
    return bsp::read_compiled_material_sort_fields(*material, *pass, fields, error);
}
bool write_mesh_instance(void* opaque, const bsp::InstanceRenderEntry& entry,
    std::uint8_t* output, std::size_t available, std::string& error) {
    if (!entry.model || !entry.model->context || !entry.section
        || !entry.section->material_clone_owner || available < sizeof(bsp::BuildingInstanceData)) return false;
    const auto& model = *static_cast<const MeshInstanceModel*>(entry.model->context);
    const auto material = std::static_pointer_cast<bsp::MaterialCloneState>(entry.section->material_clone_owner);
    bsp::BuildingInstanceData record;
    std::vector<bsp::BuildingInstancePointLight> point_lights;
    for (const auto* light : model.lifetime.point_lights_164) point_lights.push_back(light->values);
    if (!bsp::write_building_instance_data_00b55780(model.transform.world, point_lights, entry.visibility,
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
        const auto& original = *static_cast<const MeshInstanceModel*>(entry.model->context);
        auto& control = static_cast<MeshModelControl&>(original.lifetime.storage_owner);
        auto context = create_mesh_model(original.lifetime.runtime,
            original.transform.local, original.bounds.local_sphere, geometry, control.evidence);
        model.geometry = geometry.get(); model.context = context->model; model.context_owner = context;
        model.attach_scene = attach_mesh_scene; model.world_sphere_center = mesh_world_sphere;
        return true;
    }
private:
    bsp::D3D9StateCache& states_;
    std::shared_ptr<bsp::VertexBufferBinding> physical_;
    bsp::GeneratedInstanceGeometrySource source_;
    bsp::GeneratedInstanceGeometryGenerator generator_;
};

bool draw_mesh(IDirect3DDevice9& device, bsp::NativeRendererParametersOwner& renderer_parameters,
    MeshTextureDomain& textures,
    const InstalledModelProbe& model, const std::shared_ptr<bsp::CompiledMaterialPass>& material_pass, std::string& error) {
    const auto& shaders = *material_pass;
    if (shaders.base.options.instance_generator != "building" || !model.hierarchy.matrix
        || shaders.shadow_samplers.shadow_texture_slot != 1 || shaders.shadow_samplers.shadow_map_slot != -1)
        return false;
    const auto& mesh = model.mesh;
    const auto& subset = mesh.subsets[0];
    bsp::MaterialLighting lighting;
    bsp::MaterialTextureSlots slots;
    std::vector<const bsp::MeshVertexStreamPayload*> ordered_streams;
    for (const auto& event : subset.events) {
        if (const auto* selected = std::get_if<bsp::MeshVertexStreamReference>(&event)) {
            if (selected->index >= mesh.vertex_streams.size()) return false;
            ordered_streams.push_back(&mesh.vertex_streams[selected->index]);
        } else if (const auto* texture = std::get_if<bsp::MeshTextureRequest>(&event)) {
            const auto logical = textures.acquire(texture->name, error);
            if (!logical) return false;
            if (!bsp::set_material_texture_00b189f0(slots, texture->slot, logical)) return false;
        } else if (const auto* record = std::get_if<bsp::MeshLightingRecord>(&event))
            lighting.set_lighting_record_00b179d0(record->ignored_slot, record->values);
    }
    if (ordered_streams.size() != 1 || !mesh.indices) return false;
    // This white image is a controlled unoccluded ShadowTexture scene input.
    // Native shadow generation/global ownership is a separate remaining task.
    const auto shadow = textures.acquire("white.tga", error);
    if (!shadow) return false;
    bsp::CameraState camera_state;
    bsp::CameraMatrix camera_world{1,0,0,0, 0,1,0,0, 0,0,1,0, 8,8,8,1};
    const bsp::CameraMatrix desired_view_projection{.24f,-.14f,-.1f,0, 0,.28f,-.1f,0,
        -.24f,-.14f,-.1f,0, 0,0,.5f,1};
    bsp::CameraMatrix projection;
    bsp::set_camera_local_matrix_00b71430(camera_state, camera_world);
    bsp::multiply_camera_matrices_00413920(projection, camera_world, desired_view_projection);
    camera_state.projection.fov = 1; // Explicit host scalar; time prefix uses its reciprocal.
    bsp::set_camera_projection_00b6fd60(camera_state.projection, projection);
    auto& camera = camera_state.transform;
    std::vector<float> vertex_words(256 * 4), pixel_words(224 * 4);
    std::uint32_t first_constant_register = 77, vertex_header = 0, pixel_header = 0;
    bsp::MaterialEntryConstantState material_constants{first_constant_register, vertex_words, pixel_words};
    bsp::MeshDecodeBindingStats decoded;
    if (!bsp::pack_mesh_vertex_decode_constants_00b428c0(ordered_streams, shaders.vb,
            {static_cast<std::uint32_t>(shaders.base.options.compressed_element_count), {}},
            0, vertex_words, decoded, error)
        || decoded.records_from_metadata != 3 || decoded.records_written != 3) return false;
    std::array<float, 24> expected_decode_constants;
    std::memcpy(expected_decode_constants.data(),
        vertex_words.data() + shaders.vb.registers[24] * 4, sizeof(expected_decode_constants));
    bsp::BuildingInstanceData instance;
    if (!bsp::write_building_instance_data_00b55780(*model.hierarchy.matrix, {}, 1,
        lighting.diffuse_color_00b179f0(0)[3], instance, error)) return false;
    auto declaration = std::make_shared<bsp::VertexDeclaration>();
    auto instance_declaration = std::make_shared<bsp::VertexDeclaration>();
    if (!bsp::decode_vertex_format_00b2dbd0(ordered_streams[0]->format_name, *declaration, error)
        || !bsp::decode_vertex_format_00b2dbd0(bsp::building_instance_vertex_format, *instance_declaration, error)) return false;

    OwnedCom<IDirect3DStateBlock9> saved;
    OwnedCom<IDirect3DSurface9> old_target, old_depth, target, depth, readback;
    std::array<OwnedCom<IDirect3DSurface9>, 3> old_extra_targets;
    HRESULT hr = device.CreateStateBlock(D3DSBT_ALL, &saved.p);
    if (SUCCEEDED(hr)) hr = device.GetRenderTarget(0, &old_target.p);
    if (SUCCEEDED(hr)) {
        const auto result = device.GetDepthStencilSurface(&old_depth.p);
        if (FAILED(result) && result != D3DERR_NOTFOUND) hr = result;
    }
    for (UINT slot = 1; SUCCEEDED(hr) && slot < 4; ++slot) {
        const auto result = device.GetRenderTarget(slot, &old_extra_targets[slot - 1].p);
        if (FAILED(result) && result != D3DERR_NOTFOUND) hr = result;
    }
    if (FAILED(hr)) return false;
    unsigned visible = 0; std::set<DWORD> colors;
    bool buffers_match = false, constants_match = false, layout_match = false, bindings_match = false;
    bool groups_match = false, capacity_guard = false, frame_targets_match = false, camera_frame_match = false;
    MeshModelLifetimeEvidence model_lifetimes;
    unsigned queued_bindings_checked = 0;
    unsigned material_constants_built = 0;
    unsigned system_constants_built = 0, particle_clocks_destroyed = 0, type_counters_destroyed = 0;
    bool system_prefix_match = false, particle_lifetime_match = false, fog_lifetime_match = false;
    bool fog_factory_match = false;
    bool light_ownership_match = false, type_lifetime_match = false;
    try {
        bsp::RendererSynchronization sync;
        bsp::D3D9StateCache states(device, sync, nullptr);
        bsp::NativeD3D9RendererParameterDispatch parameter_dispatch(states, renderer_parameters);
        states.bind_native_renderer_parameters(&parameter_dispatch);
        bsp::D3D9StateCache* current_renderer = &states;
        bsp::D3D9CameraFrameAccess camera_access(states);
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
        if (!bsp::assign_compiled_material_effect_00b19210_fragment(
            *source_material, material_pass, error)) throw std::runtime_error(error);
        // Ordinary constructor00b18900 initializes both offset-named words
        // toFFFFFFFF; clone00b18b60 then copies them without interpretation.
        source_material->word104 = source_material->word108 = 0xffffffffu;
        auto source_geometry_owner = std::make_shared<bsp::GeneratedInstanceGeometry>();
        auto& source_geometry = *source_geometry_owner;
        source_geometry.mesh_stream = vertices; source_geometry.indices = indices;
        source_geometry.section.primitive = subset.native_primitive;
        source_geometry.section.range_words = subset.range_words;
        if (!shaders.effect_owner || !shaders.effect_owner->sort_metadata.descriptor_assigned)
            throw std::runtime_error("Installed effect has no construction/descriptor metadata");
        source_geometry.section.material_order = shaders.effect_owner->sort_metadata.priority_b0;
        source_geometry.section.material_queue_index = static_cast<std::uint32_t>(shaders.effect_owner->sort_metadata.pipe_id_ac);
        source_geometry.section.material_clone_owner = source_material;
        // Explicit distinct host type identities exercise the recovered equality
        // predicates. These are not claimed to be the native runtime ID values.
        bsp::SceneAttachmentRuntime scene_runtime(4, {1, 2, 3});
        bsp::GeneratedModelLifetimeRuntime model_runtime(scene_runtime);
        std::unique_ptr<bsp::SceneResource, ReleaseMeshScene> scene(new bsp::SceneResource(1,
            [](bsp::SceneResource& value) { delete &value; }));
        auto source_model_state = create_mesh_model(model_runtime,
            *model.hierarchy.matrix, model.hierarchy.sphere, source_geometry_owner, model_lifetimes);
        bsp::InstanceUploadModel source_model;
        source_model.geometry = &source_geometry; source_model.context = source_model_state->model;
        source_model.context_owner = source_model_state; source_model.attach_scene = attach_mesh_scene;
        source_model.world_sphere_center = mesh_world_sphere;
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
        MeshRenderCommand render_command;
        auto& groups = render_command.command.grouping;
        auto& opaque_queue = render_command.command.batches[0]->entries;
        auto& faded_queue = render_command.command.batches[1]->entries;
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
        bsp::InstanceUploadContext upload_context{groups.ordered_groups,scene.get(),&camera,queues};
        if (SUCCEEDED(hr)) hr = bsp::upload_instance_groups_00b1e990_fragment(states,upload_context,upload,error);
        if (FAILED(hr)) throw std::runtime_error("Instance grouping/upload: " + error);
        const std::vector<bsp::RenderBatchSortConfiguration> sort_configuration{{1,0},{1,1}};
        const bsp::RenderBatchMaterialKeySource material_keys{nullptr, read_mesh_material_key};
        for (std::uint32_t index = 0; index < queues.size(); ++index)
            if (!bsp::prepare_render_batch_00b51df0(*queues[index], index,
                sort_configuration, material_keys, error))
                throw std::runtime_error("Installed batch preparation: " + error);
        auto& full = groups.by_binding[0]->upload.categories[0];
        auto& faded = groups.by_binding[0]->upload.categories[1];
        const auto generated = groups.by_binding[0]->geometries[0];
        auto instances = generated->instance_stream;
        const auto faded_instances = groups.by_binding[0]->geometries[1]->instance_stream;
        const auto clone = std::static_pointer_cast<bsp::MaterialCloneState>(generated->section.material_clone_owner);
        const auto shared_effect = std::static_pointer_cast<const bsp::CompiledMaterialPass>(clone->effect)->effect_owner;
        const auto fallback_next = textures.counters.next_texture_serial20;
        const auto reused_fallback = textures.acquire("ERROR.TGA", error);
        const bool metadata_match = shared_effect == shaders.effect_owner
            && shared_effect->fallback_texture_98 == reused_fallback
            && textures.counters.next_texture_serial20 == fallback_next
            && textures.counters.next_effect_serial_c0 == 1
            && clone->textures.textures()[0]->sort_metadata.has_value();
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
            && static_cast<MeshInstanceModel*>(full.generated_model->context)->attachment.scene == scene.get()
            && static_cast<MeshInstanceModel*>(faded.generated_model->context)->attachment.scene == scene.get()
            && scene->references.load() == 3 && scene->registry.size() == 0 && metadata_match;
        std::printf("Installed batch metadata: effect_serial=%u priority=%d texture_serial=%u constructions=%u/%u shared_effect_fallback=%d key=%llu\n",
            shared_effect->sort_metadata.construction_serial_c0, shared_effect->sort_metadata.priority_b0,
            clone->textures.textures()[0]->sort_metadata->construction_serial20,
            textures.counters.next_effect_serial_c0, textures.counters.next_texture_serial20,
            metadata_match, static_cast<unsigned long long>(full.output_entry->sort_key));
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
        auto target_surfaces = std::make_shared<bsp::D3D9DefaultSurfaces>();
        auto frame_targets = std::make_shared<bsp::D3D9FrameTargets>();
        if (SUCCEEDED(hr)) hr = bsp::surface_initialize_00b3cc80(target_surfaces->color, target.p);
        if (SUCCEEDED(hr)) hr = bsp::surface_initialize_00b3cc80(target_surfaces->depth, depth.p);
        frame_targets->colors[0] = std::shared_ptr<bsp::D3D9SurfaceBinding>(target_surfaces, &target_surfaces->color);
        frame_targets->depth = std::shared_ptr<bsp::D3D9SurfaceBinding>(target_surfaces, &target_surfaces->depth);
        bsp::D3D9SurfaceBinding default_color;
        default_color.surface = old_target.p; // Retained by old_target for this whole call.
        if (SUCCEEDED(hr)) {
            const auto color_count = states.color_binding_calls(), depth_count = states.depth_binding_calls();
            // A null slot0 resolves to the retained default wrapper without being
            // counted as a non-null explicit binding.
            hr = states.bind_color_surface_00b23d80(0, nullptr, default_color);
            OwnedCom<IDirect3DSurface9> actual_default;
            if (SUCCEEDED(hr)) hr = device.GetRenderTarget(0, &actual_default.p);
            frame_targets_match = SUCCEEDED(hr) && actual_default.p == old_target.p
                && states.color_binding_calls() == color_count;
            if (SUCCEEDED(hr)) hr = states.bind_frame_targets_00b24e70(frame_targets, default_color, true);
            frame_targets_match = frame_targets_match && SUCCEEDED(hr)
                && states.frame_targets() == frame_targets.get()
                && states.color_binding_calls() == color_count + 1
                && states.depth_binding_calls() == depth_count + 1;
            if (SUCCEEDED(hr)) {
                const auto repeated = states.bind_frame_targets_00b24e70(frame_targets, default_color, true);
                const auto cleared = states.bind_frame_targets_00b24e70({}, default_color, true);
                frame_targets_match = frame_targets_match && repeated == S_FALSE && cleared == S_OK
                    && !states.frame_targets() && states.color_binding_calls() == color_count + 1
                    && states.depth_binding_calls() == depth_count + 1;
                OwnedCom<IDirect3DSurface9> retained_color, retained_depth;
                hr = device.GetRenderTarget(0, &retained_color.p);
                if (SUCCEEDED(hr)) hr = device.GetDepthStencilSurface(&retained_depth.p);
                frame_targets_match = frame_targets_match && SUCCEEDED(hr)
                    && retained_color.p == target.p && retained_depth.p == depth.p;
            }
            if (SUCCEEDED(hr)) hr = states.bind_frame_targets_00b24e70(frame_targets, default_color, true);
            for (UINT slot = 1; SUCCEEDED(hr) && slot < 4; ++slot) {
                OwnedCom<IDirect3DSurface9> actual;
                const auto result = device.GetRenderTarget(slot, &actual.p);
                frame_targets_match = frame_targets_match && result == D3DERR_NOTFOUND && !actual.p;
            }
            DWORD srgb = ~DWORD{};
            if (SUCCEEDED(hr)) hr = device.GetRenderState(D3DRS_SRGBWRITEENABLE, &srgb);
            frame_targets_match = frame_targets_match && SUCCEEDED(hr) && srgb == 0
                && states.color_binding_calls() == color_count + 2
                && states.depth_binding_calls() == depth_count + 2;
            std::printf("Installed frame targets: retained_surfaces_default_color_identity_skip_null_preserves_and_slots=1 checked=%d\n", frame_targets_match);
            if (!frame_targets_match && SUCCEEDED(hr)) hr = E_FAIL;
        }
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
        std::int32_t uploaded_vertex_registers = 0, uploaded_pixel_registers = 0;
        if (SUCCEEDED(hr) && !bsp::upload_material_entry_constants_00b43541(shaders,
            material_constants,current_renderer,hr,uploaded_vertex_registers,
            uploaded_pixel_registers,error)) hr = E_FAIL;
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
        const bsp::CameraViewport viewport{0,0,256,256,0,{0,0,256,256}};
        bsp::CameraFrameState camera_frame(camera_state);
        bsp::initialize_system_fog_camera_slot_00b71ae3(camera_frame.fog_184);
        MeshFogCameraShutdown fog_shutdown{camera_frame.fog_184};
        // Exercise the actual factory's optional-receiver/absent-record path
        // with this same diagnostic camera. Outer game ownership stays outside
        // this probe; the environment producer below supplies chosen draw values.
        bsp::FogReceiverFields* fog_receiver = nullptr;
        bsp::CameraFrameState* fog_camera_slot = &camera_frame;
        const void* fog_scene_record = nullptr;
        std::uint32_t fog_packed_color = 0;
        const bsp::WorldFogFactoryGameFields fog_game{fog_receiver,fog_camera_slot,fog_scene_record};
        bsp::initialize_world_fog_004df6a3(fog_game,fog_packed_color);
        auto* fog = bsp::system_fog_owner_from_state(camera_frame.fog_184);
        fog_factory_match = fog && fog->references_04 == 1 && fog_packed_color == 0x00a5a3ab;
        // Explicit diagnostic environment values use the recovered producer.
        // Native construction leaves directionals untouched until these writes.
        const std::array<float,11> fog_scalars{0,100,200,0,1,0,0,0,0,0,0};
        const std::array<float,4> fog_color{.35f,.35f,.35f,1}, underwater_color{};
        const std::array<std::array<float,4>,4> fog_directionals{};
        const bsp::EnvironmentFogFields fog_environment{
            fog_scalars,fog_color,fog_directionals,underwater_color};
        if (!bsp::apply_environment_fog_0078d076(fog_environment,camera_frame.fog_184,error)) hr = E_FAIL;
        camera_frame.enabled = 1; camera_frame.viewport = &viewport;
        camera_frame.clear_flags = D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER;
        camera_frame.clear_color = 0xff000000; camera_frame.clear_depth = 1;
        camera_frame.render_mode = 0; // The actual compiled NORMAL program used below.
        camera_frame.frustum.count = 6;
        camera_access.user_clip_planes_supported = true;
        camera_access.sse2_color_truncation = true; // Explicit host mode, not native startup recovery.
        if (SUCCEEDED(hr)) {
            camera_access.execute_camera_command_00b71360(camera_frame);
            D3DVIEWPORT9 actual{}; DWORD actual_ambient = 0, clip_mask = ~DWORD{}, scissor = ~DWORD{};
            hr = device.GetViewport(&actual);
            if (SUCCEEDED(hr)) hr = device.GetRenderState(D3DRS_AMBIENT, &actual_ambient);
            if (SUCCEEDED(hr)) hr = device.GetRenderState(D3DRS_CLIPPLANEENABLE, &clip_mask);
            if (SUCCEEDED(hr)) hr = device.GetRenderState(D3DRS_SCISSORTESTENABLE, &scissor);
            camera_frame_match = SUCCEEDED(hr) && actual.X == 0 && actual.Y == 0
                && actual.Width == 256 && actual.Height == 256 && actual.MinZ == 0 && actual.MaxZ == 1
                && actual_ambient == 0xff595959 && clip_mask == 0 && scissor == 0
                && camera_access.viewport() == &viewport && camera_access.viewport_calls() == 1
                && camera_access.clear_calls() == 1 && camera_access.plane_set().count == 6
                && camera_access.pending_plane_count() == 0 && camera_access.active_plane_count() == 0
                && (camera_state.projection.valid_flags & 0x24u) == 0x24u;
            std::printf("Installed camera frame: shared_camera_cache_viewport_clear_ambient_frustum=1 checked=%d\n", camera_frame_match);
            if (!camera_frame_match && SUCCEEDED(hr)) hr = E_FAIL;
        }
        // These actual allocation/lock/destruction adapters share one lifetime
        // domain and one particle slot for this isolated scene.
        bsp::SizedStoragePool particle_strings(bsp::string_pool_config_00419cc0());
        bsp::ParticleClock* particle_slot = nullptr;
        MeshParticleLifetimeCallbacks lifetime_callbacks{nullptr,particle_clocks_destroyed};
        bsp::SingletonLifetimeDomain lifetime({&lifetime_callbacks,
            &MeshParticleLifetimeCallbacks::destroy,&MeshParticleLifetimeCallbacks::invalid_parameter});
        bsp::ConcreteParticleClockLifetimeAccess particle_lifetime(lifetime,particle_slot,particle_strings);
        lifetime_callbacks.access = &particle_lifetime;
        bsp::TypeIdCounterStorage* type_slot = nullptr;
        bsp::TypeIdCounterLifetime type_lifetime(lifetime, type_slot);
        lifetime_callbacks.types = &type_lifetime;
        lifetime_callbacks.type_slot = &type_slot;
        MeshSingletonShutdown lifetime_shutdown{lifetime};
        bsp::SystemConstantPrefix system_prefix{}; // Explicit initialized host preimage.
        if (SUCCEEDED(hr) && !build_mesh_system_constants(camera_frame,current_renderer,
            particle_slot,particle_lifetime,type_lifetime,particle_strings,
            light_ownership_match,system_prefix,hr,error)) hr = E_FAIL;
        if (SUCCEEDED(hr)) {
            ++system_constants_built;
            system_prefix_match = gpu_system_prefix_matches(device,system_prefix);
            if (!system_prefix_match) hr = E_FAIL;
        }
        if (SUCCEEDED(hr)) hr = device.BeginScene();
        MeshMaterialConstantOwners material_owners(camera_frame, shadow);
        bsp::MaterialConstantBuilderEnvironment material_environment{current_renderer, nullptr,
            vertex_header, pixel_header, material_owners};
        if (SUCCEEDED(hr)) {
            for (const auto* queue : queues) for (auto* entry : queue->entries()) {
                if (FAILED(hr)) continue;
                const auto& geometry = *entry->geometry;
                const auto& section = *entry->section;
                hr = states.bind_vertex_layout_00b23f20(geometry.combined_layout);
                states.bind_index_stream_00b24b00(geometry.indices,static_cast<INT>(geometry.mesh_stream->base_vertex));
                states.bind_vertex_stream_00b24840(0,geometry.mesh_stream);
                states.bind_vertex_stream_00b24840(1,geometry.instance_stream);
                states.set_stream_frequency_00b24a40(0,geometry.mesh_stream->tag | section.instance_count);
                states.set_stream_frequency_00b24a40(1,geometry.instance_stream->tag | 1);
                // Poison just the decoded register span so the actual GPU-owner
                // builder must replace it; the earlier parsed-payload fixture
                // cannot conceal a missing runtime write.
                auto* decode_begin = vertex_words.data() + shaders.vb.registers[24] * 4;
                std::fill(decode_begin, decode_begin + expected_decode_constants.size(), -12345.0f);
                HRESULT constants_result = S_OK;
                if (SUCCEEDED(hr) && !bsp::build_material_constants_00b42350(*material_pass,
                    *entry, nullptr, material_constants, material_environment, constants_result, error)) hr = E_FAIL;
                if (SUCCEEDED(hr)) hr = constants_result;
                if (SUCCEEDED(hr)) {
                    constants_match = constants_match && std::memcmp(decode_begin,
                        expected_decode_constants.data(), sizeof(expected_decode_constants)) == 0;
                    if (!constants_match) hr = E_FAIL;
                    else ++material_constants_built;
                }
                if (SUCCEEDED(hr) && !bsp::upload_material_entry_constants_00b43541(shaders,
                    material_constants,current_renderer,hr,uploaded_vertex_registers,
                    uploaded_pixel_registers,error)) hr = E_FAIL;
                if (SUCCEEDED(hr)) {
                    system_prefix_match = system_prefix_match
                        && gpu_system_prefix_matches(device,system_prefix);
                    if (!system_prefix_match) hr = E_FAIL;
                }
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
        lifetime.shutdown();
        particle_lifetime_match = particle_slot == nullptr && particle_clocks_destroyed == 1
            && lifetime.published_manager() == nullptr;
        type_counters_destroyed = lifetime_callbacks.types_destroyed;
        type_lifetime_match = type_slot == nullptr && type_counters_destroyed == 1;
        fog_lifetime_match = camera_frame.fog_184 == &fog->fields_08 && fog->references_04 == 1;
        bsp::clear_system_fog_camera_slot_00b71f68(camera_frame.fog_184);
        fog_lifetime_match = fog_lifetime_match && camera_frame.fog_184 == nullptr;
        states.bind_vertex_shader_00b21d10(nullptr); states.bind_pixel_shader_00b21c20(nullptr);
        states.bind_texture_00b24710(0,{}); states.bind_texture_00b24710(1,{});
        states.bind_vertex_stream_00b24840(0,{}); states.bind_vertex_stream_00b24840(1,{});
        states.bind_index_stream_00b24b00({},0); states.bind_vertex_layout_00b23f20({});
        states.set_stream_frequency_00b24a40(0,1); states.set_stream_frequency_00b24a40(1,1);
        device.SetVertexDeclaration(nullptr); states.invalidate();
    } catch (const std::exception& exception) { error = exception.what(); hr = E_FAIL; }
    bool restored = SUCCEEDED(device.SetDepthStencilSurface(nullptr));
    for (UINT slot = 1; slot < 4; ++slot)
        restored = SUCCEEDED(device.SetRenderTarget(slot, nullptr)) && restored;
    restored = SUCCEEDED(device.SetRenderTarget(0,old_target.p)) && restored;
    for (UINT slot = 1; slot < 4; ++slot)
        restored = SUCCEEDED(device.SetRenderTarget(slot,old_extra_targets[slot - 1].p)) && restored;
    restored = SUCCEEDED(device.SetDepthStencilSurface(old_depth.p)) && restored;
    restored = SUCCEEDED(saved.p->Apply()) && restored;
    const bool model_lifetime_match = model_lifetimes.constructed == 3 && model_lifetimes.disposed == 3
        && model_lifetimes.cleanup_matches;
    std::printf("Installed model lifetime: constructed=%u disposed=%u recovered_group_and_virtual18_cleanup=%d checked=%d\n",
        model_lifetimes.constructed, model_lifetimes.disposed, model_lifetimes.cleanup_matches, model_lifetime_match);
    const bool checked = SUCCEEDED(hr) && buffers_match && constants_match && layout_match && bindings_match && groups_match && frame_targets_match
        && camera_frame_match && model_lifetime_match
        && system_constants_built == 1 && system_prefix_match && particle_lifetime_match && fog_lifetime_match && fog_factory_match
        && light_ownership_match && type_lifetime_match
        && material_constants_built == 2 && queued_bindings_checked == 2
        && visible > 32 && visible < 16384 && colors.size() > 10 && restored;
    std::printf("Installed material builder: actual_stream_and_clone_owners_ordered_constants=%u checked=%d\n",
        material_constants_built, material_constants_built == 2 && constants_match);
    std::printf("Installed system builder: ordered_prefix=%u VS_PS_77_retained_after_material=%d particle_clocks_destroyed=%u lifetime_checked=%d\n",
        system_constants_built,system_prefix_match,particle_clocks_destroyed,particle_lifetime_match);
    std::printf("Installed fog owner: native_constructor_environment_apply_counted_camera_clear=%d world_factory_publication_packed=%d\n",
        fog_lifetime_match,fog_factory_match);
    std::printf("Installed light owner: native_pool_types_config_canonical_registry_cleanup=%d type_counters_destroyed=%u shared_lifetime_checked=%d\n",
        light_ownership_match,type_counters_destroyed,type_lifetime_match);
    std::printf("Installed mesh draw: hr=0x%08lx vertices=%u primitives=%u GPU_bytes=%d decode_records=%zu constant_readback=%d instance_layout=%d textures_frequencies=%d queued_bindings=%u visible=%u colors=%zu restored=%d checked=%d error=%s\n",
        static_cast<unsigned long>(hr),ordered_streams[0]->count,subset.range_words[3],buffers_match,
        decoded.records_from_metadata,constants_match,layout_match,bindings_match,queued_bindings_checked,visible,colors.size(),restored,checked,error.c_str());
    return checked;
}
}

bool probe_installed_mesh(IDirect3DDevice9& device,
    bsp::NativeRendererParametersOwner& renderer_parameters, AssetStreamProbe& assets) {
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
    MeshTextureDomain textures(device, assets);
    const auto fallback = textures.acquire("error.tga", error);
    if (!fallback) { std::fprintf(stderr, "Installed error texture: %s\n", error.c_str()); return false; }
    auto effect_owner = std::make_shared<bsp::CompiledMaterialEffect>(
        bsp::construct_material_effect_sort_metadata_00b18d60(textures.counters), fallback);
    bsp::MaterialPassCompileSettings settings{0,3,false,false,effect_owner};
    std::shared_ptr<bsp::CompiledMaterialPass> shaders;
    if (!bsp::compile_material_pass(device, resolver, descriptor, settings, shaders, error)) {
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
    return draw_mesh(device, renderer_parameters, textures, model, shaders, error);
}
