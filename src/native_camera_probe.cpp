#include "native_camera_probe.hpp"
#include "bsp/native_camera_reference.hpp"
#include "bsp/native_render_context.hpp"
#include "bsp/legacy_crt_math.hpp"
#include <cstring>
#include <exception>
#include <optional>
#include <stdexcept>

namespace {
// Immutable words from the installed image, independently checked against the
// saved bsp program. These are native dispatch identities, never host addresses.
// See reports/native_camera_render_probe_audit.json for byte ranges and hashes.
constexpr std::uint32_t camera_table[22]{
    0xbd30e0,0xb71fe0,0xb6fb60,0xb71ce0,0xb6f8f0,0xb6fb70,0xb6f310,0xb6d7b0,
    0xb6fb80,0xb70de0,0xb6d820,0xb713d0,0xb71400,0xb71460,0xb71430,0xb6dbc0,
    0xb6dbe0,0xb6dc00,0xb6e8c0,0xb6dc20,0xb6ed80,0xb6ee10};
constexpr std::uint32_t node_table[22]{
    0xbd30e0,0xb6f8d0,0xb6d790,0xb6f570,0xb6f8f0,0xb6d7a0,0xb6f310,0xb6d7b0,
    0xb6d990,0xb6e150,0xb6d820,0xb6dab0,0xb6dae0,0xb6e870,0xb6db10,0xb6dbc0,
    0xb6dbe0,0xb6dc00,0xb6e8c0,0xb6dc20,0xb6ed80,0xb6ee10};
constexpr std::uint32_t context_table[2]{0xbd30e0,0xb1d570};
const volatile std::uint32_t one_00d7a24c = 0x3f800000;
const volatile std::uint32_t hundred_00ce3d08 = 0x42c80000;
const volatile std::uint32_t fov_00ce7d20 = 0x3f32b8c3;
const volatile std::uint32_t far_00d0c5f8 = 0x47435000;
const volatile std::uint32_t aspect_00d5bd98 = 0x3faaaaab;
const volatile std::uint32_t scalar_00ce77fc = 0x46ea6000;
const volatile std::uint32_t scalar_00ce3c88 = 0x44000000;
// Explicit probe CRT policy; no claim to have recovered startup dispatch mode.
const std::uint32_t crt_bypass = 1;

struct Viewports final : bsp::CameraViewportResolver {
    std::optional<bsp::CameraViewport> view;
    const bsp::CameraViewport* resolve_viewport(bsp::NativeViewportOwner* raw) override {
        if (!view || view->native_owner != raw)
            throw std::logic_error("probe viewport requires its same actual native owner");
        return &*view;
    }
};
struct PoolBinding final {
    bsp::AllocatorListDomain& domain;
    bsp::AllocatorListElement& element;
    bool active{true};
    void close() noexcept {
        if (active) { domain.unbind_virtual0(element); active = false; }
    }
    ~PoolBinding() { close(); }
};
}

struct NativeCameraProbe::Impl final : bsp::NativeRenderActualOwners {
    NativeCameraProbeEvidence& evidence;
    std::uint8_t type_guard{};
    bsp::CameraTypeDescriptor type_descriptor{};
    bsp::CameraTypeBootstrap types;
    bsp::NativeCameraPoolStorage pool_storage;
    bsp::NativeCameraPool pool;
    PoolBinding pool_binding; // after pool: also removes binding on ctor unwind
    bool pool_initialized{}, logical_released{}, camera_constructed{};
    bsp::NativeViewportEnvironment viewport_environment;
    Viewports views;
    bsp::CameraAxesCrtAccess crt{&crt_bypass, &bsp::legacy_crt_87except_00c27489};
    bsp::NativeCameraEnvironment environment;
    void* camera_raw{};
    std::unique_ptr<bsp::NativeCameraOwner> camera_owner;
    std::unique_ptr<bsp::NativeCameraReference> camera_reference;
    bsp::NativeRenderContextStorage* context_raw{};
    std::unique_ptr<bsp::NativeRenderContextReference> context_reference;

    Impl(bsp::NativeNodeDestructionRuntime& nodes, bsp::TypeIdCounterLifetime& counter,
        bsp::LightTypeBootstrap& shared_types, bsp::AllocatorListDomain& allocators,
        bsp::D3D9StateCache* const volatile& renderer,
        bsp::NativeViewportRendererAccess& renderer_access, NativeCameraProbeEvidence& proof)
        : evidence(proof), types(counter,shared_types,{type_guard,type_descriptor}),
          pool(allocators,pool_storage), pool_binding{allocators,pool_storage.allocator_00},
          viewport_environment{renderer,renderer_access,one_00d7a24c},
          environment{pool,nodes,types,viewport_environment,views,crt,
              {hundred_00ce3d08,fov_00ce7d20,far_00d0c5f8,aspect_00d5bd98,
               scalar_00ce77fc,scalar_00ce3c88},camera_table,node_table} {
        try {
            types.initialize_static_00cd7d80();
            pool.initialize_00b715f0(); pool_initialized = true;
            camera_raw = pool.allocate_raw_slot_00b71770();
            if (!camera_raw) throw std::bad_alloc();
            // Chosen deterministic probe preimage; preserve the actual pool ID.
            std::memset(camera_raw,0,bsp::NativeCameraPool::slot_slab_index_offset);
            camera_owner = std::make_unique<bsp::NativeCameraOwner>(camera_raw,
                bsp::NativeCameraPool::slot_bytes,environment);
            const bsp::NativeString empty;
            bsp::construct_native_camera_00b71a80(*camera_owner,empty);
            camera_constructed = true;
            auto& owner = *camera_owner;
            views.view.emplace(*owner.storage.camera.viewport_180);
            evidence.constructor_width = views.view->width;
            evidence.constructor_height = views.view->height;
            evidence.same_backing = &owner.camera.transform == &owner.node.transform
                && &owner.projection.valid_flags == &owner.storage.camera.valid_flags_2f0
                && &owner.frame.frustum == &owner.storage.camera.planes_2f4
                && owner.frame.viewport.get() == &*views.view;
            camera_reference = std::make_unique<bsp::NativeCameraReference>(owner,
                bsp::NativeCameraCompanionDisposal{this,retire_camera});
            evidence.creator_one = owner.storage.node.references_04 == 1
                && &camera_reference->reference_count == &owner.storage.node.references_04;

            // Explicit isolated context allocation plus the recovered18h write
            // fragment. This is not the full44h command constructor or a queue.
            void* raw = bsp::singleton_lifetime_allocate(
                {bsp::SingletonAllocationKind::object,0x18,0x18});
            if (!raw) throw std::bad_alloc();
            context_raw = bsp::initialize_native_render_context_00b1edc0_fragment(raw);
            context_reference = std::make_unique<bsp::NativeRenderContextReference>(
                *context_raw,*this,context_table,
                bsp::NativeRenderContextCompanionDisposal{this,retire_context});
            // Explicit ownership publication: raw camera identity then retain.
            context_raw->camera_08 = &owner.storage.node;
            bsp::retain_render_command_reference(*camera_reference);
            evidence.context_two = owner.storage.node.references_04 == 2
                && context_raw->camera_08 == camera_raw && context_raw->references_04 == 1;
        } catch (...) { close(); throw; }
    }
    ~Impl() { close(); }

    bsp::RenderCommandReference& resolve_actual(void* raw) override {
        if (raw == camera_raw && camera_reference && !evidence.camera_retired)
            return *camera_reference;
        if (raw == context_raw && context_reference && !evidence.context_retired)
            return *context_reference;
        throw std::logic_error("probe context has no canonical companion for raw owner");
    }
    static void retire_camera(void* context, bsp::NativeCameraReference& reference) noexcept {
        auto& self = *static_cast<Impl*>(context);
        if (reference.camera_owner().phase != bsp::NativeCameraOwner::Phase::dead)
            std::terminate();
        self.evidence.camera_retired = true;
        self.camera_raw = nullptr; // do not access returned native storage
    }
    static void retire_context(void* context, bsp::NativeRenderContextReference&) noexcept {
        auto& self = *static_cast<Impl*>(context);
        self.evidence.context_retired = true;
        self.context_raw = nullptr; // native allocation was scalar-freed
    }
    void close() noexcept {
        if (!pool_initialized) { pool_binding.close(); return; }
        if (camera_reference && !logical_released && !evidence.camera_retired) {
            logical_released = true;
            bsp::unlink_and_release_render_model_00b6dfa0(*camera_reference);
            evidence.logical_one = !evidence.camera_retired
                && camera_reference->reference_count == 1;
        }
        if (context_reference && !evidence.context_retired)
            bsp::release_render_command_reference(*context_reference);
        else if (context_raw) {
            bsp::delete_native_render_context_00b1d570(context_raw,*this,1);
            context_raw = nullptr;
        }
        context_reference.reset();
        camera_reference.reset();
        if (camera_raw) {
            if (camera_owner && camera_owner->phase == bsp::NativeCameraOwner::Phase::live) {
                bsp::delete_native_camera_00b71fe0(*camera_owner,1);
                camera_raw = nullptr;
            } else if (camera_owner && !camera_constructed
                && camera_owner->phase == bsp::NativeCameraOwner::Phase::dead) {
                // Native ctor unwind intentionally leaves a published viewport.
                // The chosen zero preimage makes an unpublished word null. Read
                // only raw bytes after typed tail lifetime ended, then dispose
                // this extra probe allocation; no native unwind parity claim.
                bsp::NativeViewportOwner* published{};
                std::memcpy(&published,static_cast<std::byte*>(camera_raw)+0x180,sizeof(published));
                if (published) bsp::release_native_viewport_owner(*published);
            }
            camera_owner.reset(); // abandon prepared views before returning raw
            if (camera_raw) pool.return_raw_slot_00b711e0(camera_raw);
            camera_raw = nullptr;
        }
        camera_owner.reset();
        views.view.reset();
        if (pool_storage.slab_count_2c == 1) {
            std::uint16_t free_count{};
            std::memcpy(&free_count,pool_storage.slabs_28[0]+0x8bc0,sizeof(free_count));
            evidence.pool_returned = free_count == bsp::NativeCameraPool::slots_per_slab;
        }
        pool.trim_empty_slabs_00b716d0();
        evidence.pool_destroyed = pool_storage.slab_count_2c == 0;
        pool.destroy_00b71120(); pool_initialized = false;
        pool_binding.close();
    }
};

bool NativeCameraProbeEvidence::checked() const noexcept {
    return same_backing && creator_one && context_two && logical_one
        && camera_retired && context_retired && pool_returned && pool_destroyed;
}
NativeCameraProbe::NativeCameraProbe(bsp::NativeNodeDestructionRuntime& nodes,
    bsp::TypeIdCounterLifetime& counter, bsp::LightTypeBootstrap& types,
    bsp::AllocatorListDomain& allocators, bsp::D3D9StateCache* const volatile& renderer,
    bsp::NativeViewportRendererAccess& renderer_access, NativeCameraProbeEvidence& evidence)
    : impl_(std::make_unique<Impl>(nodes,counter,types,allocators,renderer,renderer_access,evidence)) {}
NativeCameraProbe::~NativeCameraProbe() = default;
bsp::NativeCameraOwner& NativeCameraProbe::owner() noexcept { return *impl_->camera_owner; }
const bsp::CameraViewport& NativeCameraProbe::viewport() const noexcept { return *impl_->views.view; }
void NativeCameraProbe::close() noexcept { impl_->close(); }
