#include "bsp/native_gui_scene_acquisition_storage.hpp"
#include "bsp/native_gui_page_registry.hpp"
#include "bsp/native_node_root_propagation.hpp"
#include "bsp/native_directional_light_construction.hpp"
#include "bsp/directional_light_pool.hpp"
#include "bsp/native_camera_configuration_leaves.hpp"
#include "bsp/native_world_configuration_leaves.hpp"
#include "bsp/lighting_configuration_apply.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_ref_counted.hpp"
#include "bsp/native_render_batch_keys.hpp"
#include "bsp/system_lighting_owners.hpp"
#include <cstring>
#include <exception>
#include <new>
#include <stdexcept>
#include <type_traits>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw GUI acquisition requires MSVC Win32 x87/SSE instructions.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
Word bits(const volatile void* p) noexcept { return reinterpret_cast<Word>(p); }
void* pointer(Word p) noexcept { return reinterpret_cast<void*>(p); }
void* at(void* p, Word offset) noexcept { return pointer(bits(p) + offset); }
Word read(const void* p, Word offset = 0) noexcept {
    return *reinterpret_cast<const volatile Word*>(bits(p) + offset);
}
void write(void* p, Word offset, Word value) noexcept {
    *static_cast<volatile Word*>(at(p, offset)) = value;
}
void byte(void* p, Word offset, unsigned char value) noexcept {
    *static_cast<volatile unsigned char*>(at(p, offset)) = value;
}
bool nonzero_byte(const void* p, Word offset) noexcept {
    return *reinterpret_cast<const volatile unsigned char*>(bits(p) + offset) != 0;
}
void* local(const NativeGuiSceneAcquisitionFrame& f, Word offset) noexcept {
    return pointer(bits(f.locals) + offset - 0x10u);
}
void state(NativeGuiSceneAcquisitionAcquired& a, int value) noexcept { a.native_eh_state = value; }
void call(NativeGuiSceneAcquisitionAcquired& a, Word site) noexcept { a.active_call_site = site; }
NativeStringRawPoolContext& strings(NativeGuiSceneAcquisitionContext& c) noexcept {
    return c.camera.lifetime.node.strings;
}
void require_context(const NativeGuiSceneAcquisitionFrame& f,
    NativeGuiSceneAcquisitionContext& c, NativeGuiSceneAcquisitionAcquired& a) {
    const auto& node = c.camera.lifetime.node;
    auto& owners = c.outer.storage.owners;
    if (a.started || a.scene_construct.started || a.scene_destroy.started ||
        a.camera_construct.started || a.camera_destroy.started ||
        a.resource_construct.started || a.directional_destroy.started ||
        a.scene_reference || a.camera_reference || a.resource_reference || a.directional_reference)
        throw std::invalid_argument("GUI acquisition requires fresh persistent diagnostics");
    if (!f.locals || !f.pushed_words || !f.directional_world.words ||
        !c.gui_camera_prefix_00d5cb5c || !c.gui_lights_name_00d5cb50 || !c.gui_directional_name_00d5cb3c ||
        !c.radius.sqrt || !c.radius.sqrt->dispatch_bypass_0109dd78 ||
        !c.radius.sqrt->except_00c27489 || !c.radius.sse2_conversion_0109eea4)
        throw std::invalid_argument("GUI acquisition requires prepared scratch/current bindings");
    if (&c.manager.actual_resources != &owners || &node.scenes.owners != &owners ||
        &node.trees.owners != &owners || &c.scene_fields.owners != &owners ||
        &c.resource.storage.ambient.owners != &owners ||
        &c.resource.storage.ambient_identity.registry != &owners ||
        &c.directional.owners != &owners || &c.directional.lifetime.node != &node ||
        c.outer.storage.tree != &node.trees || &c.outer.storage.strings != &node.strings ||
        &c.resource.storage.strings != &node.strings ||
        &c.manager.actual_manager_01090aa0 != &node.strings.actual_manager_publication_01090aa0)
        throw std::invalid_argument("GUI acquisition requires SAME actual owner/string/root domain");
    if (&c.camera.lifetime.increment_00ce221c != &node.scenes.increment_00ce221c ||
        &c.camera.lifetime.increment_00ce221c != &c.scene_fields.increment_00ce221c ||
        &c.camera.lifetime.increment_00ce221c != &c.resource.storage.ambient.increment_00ce221c ||
        &c.camera.lifetime.increment_00ce221c != &c.fog.increment_00ce221c ||
        &c.camera.lifetime.decrement_00ce2220 != &node.scenes.decrement_00ce2220 ||
        &c.camera.lifetime.decrement_00ce2220 != &node.trees.decrement_00ce2220 ||
        &c.camera.lifetime.decrement_00ce2220 != &c.scene_fields.decrement_00ce2220 ||
        &c.camera.lifetime.decrement_00ce2220 != &c.outer.storage.decrement_00ce2220 ||
        &c.camera.lifetime.decrement_00ce2220 != &c.resource.storage.ambient.decrement_00ce2220 ||
        &c.camera.lifetime.decrement_00ce2220 != &c.fog.decrement_00ce2220 ||
        c.camera.lifetime.fog_profile_00d63180 != c.fog.actual_profile_00d63180)
        throw std::invalid_argument("GUI acquisition requires SAME CURRENT import/profile cells");
    const auto* one = &c.camera.node_constants.one_00d7a24c;
    if (one != &c.outer.storage.actual_one_00d7a24c || one != &c.manager.actual_one_00d7a24c ||
        one != &c.stores.actual_one_00d7a24c || one != c.resource.storage.one_00d7a24c ||
        one != &c.camera.viewport.one_bits_00d7a24c || one != &c.camera.look_at.one_00d7a24c)
        throw std::invalid_argument("GUI acquisition requires SAME live one cell");
}
void return_string(void* captured, Word size, Word getter_site, Word return_site,
    const NativeGuiSceneAcquisitionFrame& f, NativeGuiSceneAcquisitionContext& c,
    NativeGuiSceneAcquisitionAcquired& a) {
    f.pushed_words[2] = 1; f.pushed_words[1] = size; f.pushed_words[0] = bits(captured);
    call(a, getter_site);
    auto& s = strings(c);
    auto* pool = native_string_pool_get_or_create_00419cc0(
        s.actual_published_01090aa8, s.actual_manager_publication_01090aa0);
    call(a, return_site);
    return_native_string_pool_00bd1510(pool, pointer(f.pushed_words[0]),
        f.pushed_words[1], s.actual_small_returns_disabled_01090aa4);
}
void clear_color_word(void* word) noexcept {
    byte(word, 2, 0); byte(word, 1, 0); byte(word, 0, 0); byte(word, 3, 0);
}
void zero_float_argument(volatile Word* argument) noexcept {
    __asm { mov eax, argument }
    __asm { fldz }
    __asm { fstp dword ptr [eax] }
}
void prepare_directional_descriptor(void* owner) noexcept {
    static_assert(sizeof(SystemAmbientBacklinks) == 12);
    static_assert(std::is_trivially_copyable_v<SystemAmbientBacklinks>);
    std::byte preimage[12];
    std::memcpy(preimage, at(owner, 0x178), sizeof preimage);
    auto* descriptor = ::new (at(owner, 0x178)) SystemAmbientBacklinks;
    std::memcpy(descriptor, preimage, sizeof preimage);
}
class ViewportDelete final : public NativeRefCountedDeleteCalls {
public:
    explicit ViewportDelete(NativeCameraStorageLifetimeContext& c) : context_(c) {}
    void delete_vslot04(void* owner, Word profile, Word flags) override {
        if (profile != kNativeViewportVtable || !context_.viewport_profile_00d5e5f8 ||
            context_.viewport_profile_00d5e5f8[1] != 0x00b1f8f0 || flags != 1)
            throw std::invalid_argument("GUI viewport requires genuine fresh current4 B1F8F0");
        (void)delete_native_viewport_owner_00b1f8f0(static_cast<NativeViewportOwner*>(owner), flags);
    }
private:
    NativeCameraStorageLifetimeContext& context_;
};
void viewport_zero(NativeViewportOwner* captured, NativeCameraStorageLifetimeContext& c) {
    if (captured->references_04 != 0 || captured->native_vtable_00 != kNativeViewportVtable ||
        !c.viewport_profile_00d5e5f8 || c.viewport_profile_00d5e5f8[0] != 0x00bd30e0)
        throw std::invalid_argument("GUI viewport requires SAME count0/current0 BD30E0");
    ViewportDelete calls(c);
    invoke_native_ref_counted_delete_00bd30e0(captured, calls);
}
void __fastcall bounds_call(void* actual, void*, const void* sphere) {
    set_native_gui_group_bounds_00b8e6c0(actual, sphere);
}
struct RadiusInputs {
    const volatile Word* half;
    const volatile double* squared;
    const CameraAxesCrtAccess* sqrt;
    const volatile Word* conversion;
    volatile Word* spill;
    volatile Word* sphere;
};
static_assert(sizeof(RadiusInputs) == 24);
// Source ECX=page,EDX=immutable pointer metadata. Uses existing CRT kernels;
// no float-return wrapper hides original scratch stores/current4C capture.
__declspec(naked) void __fastcall radius_tail(void*, const RadiusInputs*) {
    __asm {
        push esi
        push edi
        push ebx
        mov esi, ecx
        mov edi, edx
        mov eax, [edi]
        movss xmm0, dword ptr [eax]
        mov eax, [edi + 4]
        fld qword ptr [eax]
        mov ebx, [edi + 20]
        movss dword ptr [ebx], xmm0
        movss dword ptr [ebx + 4], xmm0
        xorps xmm0, xmm0
        movss dword ptr [ebx + 8], xmm0
        mov ecx, [edi + 8]
        call native_crt_sqrt_st0_00bf7030
        mov edx, [edi + 16]
        fstp dword ptr [edx]
        fld dword ptr [edx]
        mov ecx, [edi + 12]
        call native_crt_truncate_st0_00bf7420
        mov ecx, [esi + 4ch]
        cvtsi2ss xmm0, eax
        push ebx
        movss dword ptr [ebx + 0ch], xmm0
        xor edx, edx
        call bounds_call
        pop ebx
        pop edi
        pop esi
        ret
    }
}
void unwind(const NativeGuiSceneAcquisitionFrame& f, NativeGuiSceneAcquisitionContext& c,
    NativeGuiSceneAcquisitionAcquired& a) {
    // DEF8F8: previous state/action. Consume BEFORE action, never retry a
    // throwing cleanup. Native private EBP/handler transport is not reproduced.
    static constexpr int previous[]{-1,-1,1,2,-1,4,-1,6,-1,-1,-1,-1,11,-1};
    while (a.native_eh_state >= 0) {
        const int old = a.native_eh_state;
        state(a, previous[old]);
        Word mask = 0, header_offset = 0;
        switch (old) {
        case 0: case 6: case 9: case 10: {
            const Word offset = (old == 0 || old == 6) ? 0x14u : 0x10u;
            void* captured = pointer(read(local(f, offset)));
            a.last_cleanup_pointer = captured;
            call(a, old == 0 ? 0x00cb88e7 : old == 6 ? 0x00cb8941 :
                old == 9 ? 0x00cb896e : 0x00cb897c);
            c.resource.storage.registry_storage.actual_00bf65ac(captured);
            break;
        }
        case 1:
            a.last_cleanup_pointer = pointer(read(local(f, 0x14)));
            call(a, 0x00cb88f4);
            c.camera.lifetime.camera_pool_0108ffb0.return_raw_slot_00b711e0(a.last_cleanup_pointer);
            break;
        case 11:
            a.last_cleanup_pointer = pointer(read(local(f, 0x18)));
            call(a, 0x00cb8989);
            c.directional.pool_01090154.return_raw_slot_00b7b2f0(a.last_cleanup_pointer);
            break;
        case 2: case 4: mask=1; header_offset=0x18; call(a,0x00cb8915); break;
        case 3: case 5: mask=2; header_offset=0x30; call(a,0x00cb8934); break;
        case 7: case 8: mask=4; header_offset=0x28; call(a,0x00cb8961); break;
        case 12: case 13: mask=8; header_offset=0x30; call(a,0x00cb89a7); break;
        }
        if (mask && (f.locals[0] & mask)) {
            f.locals[0] = f.locals[0] & ~mask;
            destroy_native_string_header_0041dd20(local(f, header_offset), strings(c));
        }
        ++a.completed_unwind_actions;
    }
}
} // namespace

void acquire_native_gui_scene_storage_00ac59a0(void* page,
    const NativeGuiSceneAcquisitionFrame& f, NativeGuiSceneAcquisitionContext& c,
    NativeGuiSceneAcquisitionAcquired& a) {
    require_context(f, c, a);
    a.started = true;
    Word mask = 0; // captured EBX; distinct from callback-mutable localF10
    try {
        f.locals[0] = 0;
        call(a, 0x00ac59c5);
        void* manager = get_native_gui_manager_004c12b0(c.manager);
        call(a, 0x00ac59d3);
        void* found = find_native_gui_camera_store_00aa3280(manager, at(page, 0x108),
            f.find, c.stores.actual_empty_00f8bc60);
        const bool reuse_gate = nonzero_byte(page, 0x120);
        write(page, 0xf0, bits(found));
        if (reuse_gate && found) {
            byte(page, 0xf4, 0);
            void* captured = pointer(read(found, 0x1c));
            write(page, 0xec, bits(captured));
            call(a, 0x00ac59fd);
            c.camera.lifetime.increment_00ce221c(static_cast<volatile long*>(at(captured, 4)));
            a.reused_store = a.complete = true;
            return;
        }
        byte(page, 0xf4, 1);
        call(a, 0x00ac5a23);
        void* raw = c.resource.storage.registry_storage.actual_00bf681b(0x24);
        f.locals[1] = bits(raw); a.scene_allocation = raw; state(a, 0);
        void* scene = nullptr;
        if (raw) {
            f.scene_construct.name_argument = bits(at(page, 0x100));
            call(a, 0x00ac5a43);
            scene = construct_native_gui_scene_storage_00b724e0(raw, f.scene_construct,
                c.outer.storage, a.scene_construct);
        }
        a.scene_result = scene; state(a, -1);
        write(page, 0xec, bits(scene)); a.scene_published = true;
        if (scene) {
            a.failure = NativeGuiSceneAcquisitionAcquired::Failure::host_admission;
            a.host_admission_owner = scene;
            a.scene_reference.emplace(scene, c.outer, f.scene_destroy, a.scene_destroy);
            a.failure = NativeGuiSceneAcquisitionAcquired::Failure::none;
        }
        call(a, 0x00ac5a64);
        raw = c.camera.lifetime.camera_pool_0108ffb0.allocate_raw_slot_00b71770();
        f.locals[1] = bits(raw); a.camera_allocation = raw; state(a, 1);
        void* camera = nullptr;
        if (raw) {
            call(a, 0x00ac5a88);
            void* prefix = construct_native_string_header_0041e870(local(f, 0x18),
                strings(c), c.gui_camera_prefix_00d5cb5c);
            state(a, 2); mask = 1; a.register_mask = mask; f.locals[0] = mask;
            call(a, 0x00ac5aa7);
            void* name = concatenate_native_string_headers_004261a0(prefix,
                local(f, 0x30), at(page, 0x100), strings(c));
            mask = 3; a.register_mask = mask; f.pushed_words[2] = bits(name);
            state(a, 3); f.locals[0] = mask; call(a, 0x00ac5abf);
            camera = construct_native_camera_storage_00b71a80(raw, NativeCameraPool::slot_bytes,
                f.pushed_words[2], f.camera_construct, c.camera, a.camera_construct);
        }
        a.camera_result = camera; state(a, 4);
        if (mask & 2u) {
            void* captured = pointer(read(local(f, 0x34)));
            mask &= ~2u; a.register_mask = mask; f.locals[0] = mask;
            if (captured) return_string(captured, read(local(f, 0x30)) + 1u,
                0x00ac5af4, 0x00ac5afb, f, c, a);
        }
        state(a, -1);
        if (mask & 1u) {
            void* captured = pointer(read(local(f, 0x1c)));
            mask &= ~1u; a.register_mask = mask; // native does not rewrite localF10
            if (captured) return_string(captured, read(local(f, 0x18)) + 1u,
                0x00ac5b26, 0x00ac5b2d, f, c, a);
        }
        if (camera) {
            a.failure = NativeGuiSceneAcquisitionAcquired::Failure::host_admission;
            a.host_admission_owner = camera;
            a.camera_reference.emplace(*std::launder(static_cast<NativeNodeStorage*>(camera)),
                NativeCameraPool::slot_bytes, c.camera.lifetime, f.camera_destroy, a.camera_destroy);
            a.failure = NativeGuiSceneAcquisitionAcquired::Failure::none;
        }
        call(a, 0x00ac5b32); manager = get_native_gui_manager_004c12b0(c.manager);
        void* current_scene = pointer(read(page, 0xec));
        f.create.descriptor_argument_3c = at(page, 0x108);
        f.create.scene_argument_38 = current_scene; f.create.camera_argument_34 = camera;
        call(a, 0x00ac5b48);
        void* store = create_native_gui_camera_store_00aa5070(manager, f.create, c.stores, a.store);
        write(page, 0xf0, bits(store)); a.store_published = true;
        call(a, 0x00ac5b55); raw = c.resource.storage.registry_storage.actual_00bf681b(0x3c);
        f.locals[1] = bits(raw); a.resource_allocation = raw; state(a, 6);
        void* resource = nullptr;
        if (raw) {
            call(a, 0x00ac5b7b);
            construct_native_string_header_0041e870(local(f, 0x28), strings(c), c.gui_lights_name_00d5cb50);
            mask |= 4u; a.register_mask = mask;
            f.resource_construct.name_or_ambient_argument = bits(local(f, 0x28));
            state(a, 7); f.locals[0] = mask; call(a, 0x00ac5b96);
            resource = construct_native_scene_resource_00b83c50(raw, f.resource_construct,
                c.resource.storage, a.resource_construct);
        }
        a.resource_result = resource; f.locals[1] = bits(resource); state(a, -1);
        if (mask & 4u) {
            void* captured = pointer(read(local(f, 0x2c)));
            mask &= ~4u; a.register_mask = mask;
            if (captured) return_string(captured, read(local(f, 0x28)) + 1u,
                0x00ac5bd5, 0x00ac5bdc, f, c, a);
        }
        if (resource) {
            a.failure = NativeGuiSceneAcquisitionAcquired::Failure::host_admission;
            a.host_admission_owner = resource;
            a.resource_reference.emplace(resource, c.resource);
            a.failure = NativeGuiSceneAcquisitionAcquired::Failure::none;
        }
        current_scene = pointer(read(page, 0xec)); f.pushed_words[2] = bits(resource);
        call(a, 0x00ac5be8);
        set_native_gui_scene_resource_00b723f0(current_scene, f.pushed_words[2], c.scene_fields);
        call(a, 0x00ac5bf1);
        if (c.camera.lifetime.decrement_00ce2220(static_cast<volatile long*>(at(resource, 4))) == 0) {
            call(a, 0x00ac5c01); a.resource_reference->release_zero_references();
        }
        a.resource_creator_released = true;
        current_scene = pointer(read(page, 0xec)); call(a, 0x00ac5c0c);
        propagate_native_node_root_storage_00b6d890(camera, current_scene, c.camera.lifetime.node.scenes.children);
        a.camera_root_complete = true;
        f.pushed_words[2] = 7; call(a, 0x00ac5c15);
        set_native_camera_clear_flags_00b6fe10(camera, pointer(bits(&f.pushed_words[2])));
        clear_color_word(local(f, 0x10)); f.pushed_words[2] = f.locals[0]; call(a, 0x00ac5c35);
        set_native_camera_clear_color_00b6fe50(camera, nullptr, f.pushed_words[2]);
        call(a, 0x00ac5c3c); raw = c.resource.storage.registry_storage.actual_00bf681b(0x34);
        f.locals[0] = bits(raw); a.viewport_allocation = raw; state(a, 9);
        NativeViewportOwner* viewport = nullptr;
        if (raw) { call(a, 0x00ac5c59); viewport = initialize_native_viewport_owner_00b1f850(raw, c.camera.viewport); }
        a.viewport_result = viewport; f.pushed_words[2] = bits(viewport); state(a, -1);
        call(a, 0x00ac5c72); set_native_camera_viewport_storage_00b71990(camera, f.pushed_words[2], c.camera.lifetime);
        call(a, 0x00ac5c7b);
        if (c.camera.lifetime.decrement_00ce2220(&viewport->references_04) == 0) {
            call(a, 0x00ac5c8b); viewport_zero(viewport, c.camera.lifetime);
        }
        a.viewport_creator_released = true;
        call(a, 0x00ac5c92); raw = c.resource.storage.registry_storage.actual_00bf681b(0x94);
        f.locals[0] = bits(raw); a.fog_allocation = raw; state(a, 10);
        SystemFogOwner* fog = nullptr;
        if (raw) { call(a, 0x00ac5caf); fog = initialize_system_fog_owner_00b84e50(raw, c.fog_constants); }
        a.fog_result = fog; zero_float_argument(&f.pushed_words[2]); state(a, -1);
        call(a, 0x00ac5ccd); set_system_fog_scalar_68_00b84d00(*fog, f.pushed_words[2]);
        zero_float_argument(&f.pushed_words[2]); call(a, 0x00ac5cda);
        set_system_fog_scalar_78_00b84d40(*fog, f.pushed_words[2]);
        f.pushed_words[2] = bits(fog); call(a, 0x00ac5ce2);
        set_native_camera_fog_storage_00b71940(camera, f.pushed_words[2], c.fog);
        call(a, 0x00ac5ceb);
        if (c.camera.lifetime.decrement_00ce2220(&fog->references_04) == 0) {
            call(a, 0x00ac5cfb); invoke_native_system_fog_zero(fog, c.fog);
        }
        a.fog_creator_released = true;
        call(a, 0x00ac5d02); raw = c.directional.pool_01090154.allocate_raw_slot_00b7bac0();
        f.locals[2] = bits(raw); a.directional_allocation = raw; state(a, 11);
        void* light = nullptr;
        if (raw) {
            call(a, 0x00ac5d25);
            construct_native_string_header_0041e870(local(f, 0x30), strings(c), c.gui_directional_name_00d5cb3c);
            mask |= 8u; a.register_mask = mask; f.pushed_words[2] = bits(local(f, 0x30));
            state(a, 12); f.locals[0] = mask; call(a, 0x00ac5d40);
            light = construct_native_directional_light_raw_00b7c6b0(raw, DirectionalLightPool::slot_bytes,
                pointer(f.pushed_words[2]), strings(c), c.camera.node_constants, c.sixty_four_00ce7820);
        }
        a.directional_result = light; state(a, -1);
        if (mask & 8u) {
            void* captured = pointer(read(local(f, 0x34)));
            if (captured) return_string(captured, read(local(f, 0x30)) + 1u,
                0x00ac5d6e, 0x00ac5d75, f, c, a); // native leaves mask8 set
        }
        if (light) {
            prepare_directional_descriptor(light); a.directional_descriptor_prepared = true;
            a.failure = NativeGuiSceneAcquisitionAcquired::Failure::host_admission;
            a.host_admission_owner = light;
            a.directional_reference.emplace(light, c.directional, f.directional_destroy, a.directional_destroy);
            a.failure = NativeGuiSceneAcquisitionAcquired::Failure::none;
        }
        const Word one = c.camera.node_constants.one_00d7a24c;
        const Word profile = read(light);
        if (profile != 0x00d62fb0 || !c.directional.actual_profile_00d62fb0)
            throw std::invalid_argument("GUI directional lacks its actual current profile");
        const Word target34 = c.directional.actual_profile_00d62fb0[0x34 / 4];
        f.pushed_words[2] = bits(local(f, 0x50));
        for (unsigned i = 0; i < 16; ++i) f.locals[16 + i] = (i % 5 == 0) ? one : 0;
        call(a, 0x00ac5e00);
        if (target34 != 0x00b6e870) throw std::invalid_argument("GUI directional current34 has no genuine binding");
        set_native_raw_world_matrix_00b6e870(light, f.pushed_words[2],
            f.directional_world, c.camera.lifetime.node.world);
        const Word current_one = c.camera.node_constants.one_00d7a24c;
        f.pushed_words[2] = bits(local(f, 0x18));
        write(light, 0x1e0, 0); write(light, 0x1e4, 0); write(light, 0x1e8, current_one);
        f.locals[2] = 0; f.locals[3] = 0; f.locals[4] = 0; f.locals[5] = current_one;
        call(a, 0x00ac5e44); set_lighting_base_diffuse_004b62e0(light, pointer(f.pushed_words[2]));
        current_scene = pointer(read(page, 0xec)); call(a, 0x00ac5e52);
        propagate_native_node_root_storage_00b6d890(light, current_scene, c.camera.lifetime.node.scenes.children);
        a.directional_root_complete = true;
        const Word half = c.manager.actual_half_00ce3800;
        void* captured_resource = pointer(f.locals[1]);
        void* ambient = pointer(read(captured_resource, 0x10));
        f.locals[8] = half; f.locals[9] = half; f.locals[10] = half;
        const Word alpha = c.camera.node_constants.one_00d7a24c;
        f.pushed_words[2] = bits(local(f, 0x30)); f.locals[11] = alpha;
        call(a, 0x00ac5e8b); set_lighting_ambient_00b7af20(ambient, pointer(f.pushed_words[2]));
        const Word flags = read(page, 0x108);
        void* current_store = pointer(read(page, 0xf0));
        f.pushed_words[2] = flags | 6u;
        void* current_camera = pointer(read(current_store, 0x18));
        clear_color_word(local(f, 0x10)); call(a, 0x00ac5eb7);
        set_native_camera_clear_flags_00b6fe10(current_camera, pointer(bits(&f.pushed_words[2])));
        const Word color = f.locals[0]; current_store = pointer(read(page, 0xf0));
        current_camera = pointer(read(current_store, 0x18));
        f.pushed_words[2] = color; call(a, 0x00ac5eca);
        set_native_camera_clear_color_00b6fe50(current_camera, nullptr, f.pushed_words[2]);
        const Word priority = read(page, 0x118);
        if (read(page, 0xfc) != priority) {
            write(page, 0xfc, priority); call(a, 0x00ac5ee6);
            manager = get_native_gui_manager_004c12b0(c.manager);
            call(a, 0x00ac5eed); register_native_gui_page_00aa52a0(manager, page);
        }
        if (!nonzero_byte(page, 0x74)) {
            // Native popped EDI/EBP: sphere nowF40 (not F38); spill isF14.
            const RadiusInputs inputs{&c.manager.actual_half_00ce3800,
                &c.squared_radius_00d7a308, c.radius.sqrt, c.radius.sse2_conversion_0109eea4,
                &f.locals[1], &f.locals[12]};
            call(a, 0x00ac5f1b); radius_tail(page, &inputs);
        }
        a.complete = true;
    } catch (...) {
        if (a.resource_construct.failure == NativeSceneResourceAcquired::Failure::host_metadata_admission) {
            // Its established source contract consumed registry/name/base
            // cleanup before propagation. Keep the outer state/header/raw
            // allocation and ambient credit; do not project another unwind.
            a.nested_resource_host_prefix_cleaned = true;
            a.host_admission_owner = a.resource_construct.ambient_creator;
            a.failure = NativeGuiSceneAcquisitionAcquired::Failure::host_admission;
            throw;
        }
        if (a.failure == NativeGuiSceneAcquisitionAcquired::Failure::host_admission) throw;
        a.failure = NativeGuiSceneAcquisitionAcquired::Failure::native_call;
        a.exception_cleanup_started = true;
        try { unwind(f, c, a); } catch (...) { std::terminate(); }
        throw;
    }
}
} // namespace bsp
