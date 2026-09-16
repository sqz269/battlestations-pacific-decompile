#include "bsp/native_renderer_debug_records40.hpp"
#include "bsp/native_render_entry_cache.hpp"
#include "bsp/native_camera_view_set.hpp"
#include "bsp/native_node_raw_transform.hpp"
#include "bsp/native_renderer_container_lifetime.hpp"
#include "bsp/gui_widget_owner.hpp"
#include "bsp/native_mesh_remaining_fields.hpp"
#include "bsp/native_string.hpp"
#include <cstddef>
#include <cstring>
#include <exception>
#include <stdexcept>
#include <utility>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native debug records40 requires MSVC Win32 x87/SSE.
#endif

namespace bsp {
namespace {
using U = std::uint32_t;
static_assert(sizeof(void*) == 4);
static_assert(offsetof(NativeDebugRecords40VertexContext, actual_decrement_00ce2220) == 8);
template<class T> T read(const void* p, U offset = 0) noexcept {
    return *reinterpret_cast<const volatile T*>(reinterpret_cast<U>(p) + offset);
}
void* at(const void* p, U offset = 0) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<U>(p) + offset);
}
void put(void* p, U offset, U value) noexcept {
    *static_cast<volatile U*>(at(p, offset)) = value;
}
void require(bool condition, const char* why) {
    if (!condition) throw std::logic_error(why);
}
const volatile void* cell(const volatile void* p) noexcept { return p; }

void __fastcall dispatch_record_zero(void* actual, const NativeDebugRecords40VertexContext* c) {
    const U token = read<U>(actual); // B2B88B, only after decrement returned zero.
    for (U i = 0; i != c->terminal_count; ++i) {
        const auto& binding = c->terminals[i];
        if (binding.profile_token != token) continue;
        require(binding.actual_profile != nullptr, "records40 zero terminal requires its actual profile");
        const U target = binding.actual_profile[0]; // B2B88D current virtual0 capture.
        require(target != 0 && target == binding.expected_virtual0 && binding.invoke,
            "records40 zero terminal requires its asserted genuine current provider");
        binding.invoke(binding.context, actual);
        return;
    }
    throw std::logic_error("records40 zero terminal has an unsupported current profile");
}

void* geometry(void* model) noexcept {
    return gui_model_geometry_00b74640(*static_cast<NativeModelTailStorage*>(at(model, 0x174)), 0);
}
void* stream(void* model) noexcept {
    return native_mesh_vertex_stream_unchecked_00b73260(geometry(model), 0);
}
void* section(void* model) noexcept { return gui_geometry_element_00b732c0(geometry(model), 0); }
void stream_slot(void* actual, const NativeRendererDebugRecords40Context& c, U slot, U target) {
    const U token = read<U>(actual);
    const auto* table = c.generated.model_and_geometry.streams.actual_logical_profile_00d61d6c;
    require(token == 0x00d61d6c && table && table[slot / 4] == target,
        "records40 requires the current actual logical vertex slot");
}
void identity(CameraMatrix& matrix, const volatile U& actual_one) {
    const U one = actual_one;
    for (U i = 0; i != 16; ++i) put(matrix.data(), i * 4, i % 5 == 0 ? one : 0);
}
void make_cached_model(void* renderer, NativeRendererDebugRecords40Context& c,
    NativeRendererGeneratedModelAcquired& acquired) {
    auto& pool = c.raw_models.names;
    NativeString effect, layout, name;
    int state = -1;
    try {
        effect.assign_0041e870(pool, "debugshader.mshd"); state = 0;
        layout.assign_0041e870(pool, "pf43cc.mvfm"); state = 1;
        name.assign_0041e870(pool, "2DSprites"); state = 2;
        void* model = create_native_renderer_generated_model_00b4c700(
            name, layout, effect, 4, 0, 0, 0, c.generated, acquired, c.raw_models);
        put(renderer, 0x19e8, reinterpret_cast<U>(model));
        state = 1; destroy_native_string_header_0041dd20(&name, pool);
        state = 0; destroy_native_string_header_0041dd20(&layout, pool);
        state = -1; destroy_native_string_header_0041dd20(&effect, pool);
    } catch (...) {
        try {
            if (state >= 2) destroy_native_string_header_0041dd20(&name, pool);
            if (state >= 1) destroy_native_string_header_0041dd20(&layout, pool);
            if (state >= 0) destroy_native_string_header_0041dd20(&effect, pool);
        } catch (...) { std::terminate(); }
        throw;
    }
    // One is captured before the current +19E8 model, as at B2B684/B68C.
    const U one = c.raw_models.constants.one_00d7a24c;
    void* model = read<void*>(renderer, 0x19e8);
    CameraMatrix matrix;
    for (U i = 0; i != 16; ++i) put(matrix.data(), i * 4, i % 5 == 0 ? one : 0);
    const U token = read<U>(model);
    const auto* table = c.generated.model_and_geometry.models.vtable_00d62de8;
    require(token == 0x00d62de8 && table && table[0x38 / 4] == 0x00b6db10,
        "records40 cached model requires its current local-matrix virtual38");
    set_raw_local_matrix_00b6db10(model, c.render_entry.services, matrix.data());
}
} // namespace

__declspec(naked) void __fastcall write_native_debug_records40_vertices_00b2b741(
    void*, void*, const NativeDebugRecords40VertexContext*) {
    __asm {
        push ebp
        push ebx
        push edi
        push esi
        sub esp, 40h
        mov edi, ecx
        mov ecx, edx
        xor ebp, ebp
        mov dword ptr [esp + 24h], ebp
        cmp dword ptr [edi + 1d1ch], ebp
        jle vertex_done
        mov eax, dword ptr [esp + 54h]
        mov dword ptr [esp], eax
        mov eax, dword ptr [esp] // 00B2B741
        mov eax, dword ptr [eax]
        fld qword ptr [eax]
        mov eax, dword ptr [esp] // 00B2B747
        mov eax, dword ptr [eax + 4]
        movss xmm0, dword ptr [eax]
        fld1  // 00B2B74F
        lea esi, [ecx + 0x18] // 00B2B751
rb2b754:
        mov edx, dword ptr [edi + 0x1d18] // 00B2B754
        fld dword ptr [edx + ebp + 4] // 00B2B75A
        lea eax, [edx + ebp] // 00B2B75E
        fmul st(0), st(2) // 00B2B761
        add ecx, 0x60 // 00B2B763
        add esi, 0x60 // 00B2B766
        fsub st(0), st(1) // 00B2B769
        fstp dword ptr [esp + 0x10] // 00B2B76B
        fld dword ptr [eax + 8] // 00B2B76F
        movss xmm2, dword ptr [esp + 0x10] // 00B2B772
        fsubr st(0), st(1) // 00B2B778
        fmul st(0), st(2) // 00B2B77A
        fsub st(0), st(1) // 00B2B77C
        fstp dword ptr [esp + 0x14] // 00B2B77E
        fld dword ptr [eax + 0xc] // 00B2B782
        movss xmm1, dword ptr [esp + 0x14] // 00B2B785
        fmul st(0), st(2) // 00B2B78B
        fld dword ptr [esp + 0x10] // 00B2B78D
        fld st(0) // 00B2B791
        faddp st(2), st(0) // 00B2B793
        fxch st(1) // 00B2B795
        fstp dword ptr [esp + 0x18] // 00B2B797
        fld dword ptr [esp + 0x14] // 00B2B79B
        movss xmm3, dword ptr [esp + 0x18] // 00B2B79F
        fld dword ptr [eax + 0x10] // 00B2B7A5
        movss dword ptr [esi - 0x70], xmm0 // 00B2B7A8
        fmul st(0), st(4) // 00B2B7AD
        fsubp st(1), st(0) // 00B2B7AF
        fstp dword ptr [esp + 0x1c] // 00B2B7B1
        fstp dword ptr [ecx - 0x60] // 00B2B7B5
        fld dword ptr [esp + 0x1c] // 00B2B7B8
        fst dword ptr [esi - 0x74] // 00B2B7BC
        movss dword ptr [esi - 0x68], xmm2 // 00B2B7BF
        fld dword ptr [esp + 0x18] // 00B2B7C4
        movss dword ptr [esi - 0x64], xmm1 // 00B2B7C8
        movss dword ptr [esi - 0x60], xmm0 // 00B2B7CD
        fstp dword ptr [esi - 0x58] // 00B2B7D2
        movss dword ptr [esi - 0x50], xmm0 // 00B2B7D5
        fstp dword ptr [esi - 0x54] // 00B2B7DA
        movss dword ptr [esi - 0x48], xmm3 // 00B2B7DD
        movss dword ptr [esi - 0x44], xmm1 // 00B2B7E2
        movss dword ptr [esi - 0x40], xmm0 // 00B2B7E7
        movss dword ptr [esi - 0x38], xmm3 // 00B2B7EC
        movss xmm3, dword ptr [esp + 0x1c] // 00B2B7F1
        movss dword ptr [esi - 0x34], xmm3 // 00B2B7F7
        movss dword ptr [esi - 0x30], xmm0 // 00B2B7FC
        movss dword ptr [esi - 0x28], xmm2 // 00B2B801
        movss dword ptr [esi - 0x24], xmm1 // 00B2B806
        movss dword ptr [esi - 0x20], xmm0 // 00B2B80B
        mov eax, dword ptr [edi + 0x1d18] // 00B2B810
        mov edx, dword ptr [eax + ebp + 0x14] // 00B2B816
        mov dword ptr [esi - 0x6c], edx // 00B2B81A
        mov eax, dword ptr [edi + 0x1d18] // 00B2B81D
        mov edx, dword ptr [eax + ebp + 0x14] // 00B2B823
        mov dword ptr [esi - 0x5c], edx // 00B2B827
        mov eax, dword ptr [edi + 0x1d18] // 00B2B82A
        mov edx, dword ptr [eax + ebp + 0x14] // 00B2B830
        mov dword ptr [esi - 0x4c], edx // 00B2B834
        mov eax, dword ptr [edi + 0x1d18] // 00B2B837
        mov edx, dword ptr [eax + ebp + 0x14] // 00B2B83D
        mov dword ptr [esi - 0x3c], edx // 00B2B841
        mov eax, dword ptr [edi + 0x1d18] // 00B2B844
        mov edx, dword ptr [eax + ebp + 0x14] // 00B2B84A
        mov dword ptr [esi - 0x2c], edx // 00B2B84E
        mov eax, dword ptr [edi + 0x1d18] // 00B2B851
        mov edx, dword ptr [eax + ebp + 0x14] // 00B2B857
        mov dword ptr [esi - 0x1c], edx // 00B2B85B
        mov ebx, dword ptr [edi + 0x1d18] // 00B2B85E
        mov eax, dword ptr [ebx + ebp] // 00B2B864
        add ebx, ebp // 00B2B867
        test eax, eax // 00B2B869
        mov dword ptr [esp + 0x18], ecx // 00B2B86B
        mov dword ptr [esp + 0x1c], eax // 00B2B86F
        je rb2b8ad // 00B2B873
        add eax, 4 // 00B2B875
        fstp st(1) // 00B2B878
        push eax // 00B2B87A
        fstp st(0) // 00B2B87B
        mov eax, dword ptr [esp + 4] // 00B2B87D
        mov eax, dword ptr [eax + 8]
        call dword ptr [eax]
        test eax, eax // 00B2B883
        jne rb2b891 // 00B2B885
        mov ecx, dword ptr [esp + 0x1c] // 00B2B887
        mov edx, dword ptr [esp] // 00B2B88B
        call dispatch_record_zero
rb2b891:
        fld1  // 00B2B891
        mov eax, dword ptr [esp] // 00B2B893
        mov eax, dword ptr [eax + 4]
        movss xmm0, dword ptr [eax]
        mov eax, dword ptr [esp] // 00B2B89B
        mov eax, dword ptr [eax]
        fld qword ptr [eax]
        mov ecx, dword ptr [esp + 0x18] // 00B2B8A1
        mov dword ptr [ebx], 0 // 00B2B8A5
        fxch st(1) // 00B2B8AB
rb2b8ad:
        mov eax, dword ptr [esp + 0x24] // 00B2B8AD
        add eax, 1 // 00B2B8B1
        add ebp, 0x28 // 00B2B8B4
        cmp eax, dword ptr [edi + 0x1d1c] // 00B2B8B7
        mov dword ptr [esp + 0x24], eax // 00B2B8BD
        jl rb2b754 // 00B2B8C1
        fstp st(1) // 00B2B8CB
        fstp st(0) // 00B2B8CD
vertex_done:
        add esp, 40h
        pop esi
        pop edi
        pop ebx
        pop ebp
        ret 4
    }
}

NativeRendererDebugRecords40Frame::~NativeRendererDebugRecords40Frame() noexcept {
    if (phase_ != Phase::fresh && phase_ != Phase::retired) std::terminate();
}
void NativeRendererDebugRecords40Frame::prepare(NativeRendererDebugRecords40Context& c) {
    require(phase_ == Phase::fresh, "records40 frame must be fresh and persistent");
    auto& models = c.generated.model_and_geometry.models;
    auto& names = models.nodes.require_raw_name_pool();
    require(!models.actual_names && &names == &c.raw_models.names &&
        &models.retained_owners == &c.generated.model_and_geometry.geometry.actual_owners() &&
        &names.actual_manager_publication_01090aa0 == &c.generated.actual_manager_01090aa0 &&
        &c.cameras.nodes == &models.nodes && &c.cameras.viewport_views == &c.viewports,
        "records40 requires the same raw names, node runtime and viewport resolver");
    require(cell(&c.generated.actual_renderer_00f8d394) == cell(&c.mapping.actual_renderer_00f8d394) &&
        cell(&c.generated.actual_renderer_00f8d394) == cell(&c.pass.actual_renderer_00f8d394) &&
        cell(&c.generated.actual_renderer_00f8d394) == cell(c.gather.actual_renderer_00f8d394) &&
        c.render_entry.mapping == &c.mapping && c.gather.actual_synchronization == &c.mapping.actual_synchronization_0108d6dc &&
        &c.pass.synchronization == &c.mapping.actual_synchronization_0108d6dc &&
        &c.actual_increment_00ce221c == &c.pass.actual_increment_00ce221c &&
        cell(c.render_entry.entry_cache_0108fe88) == cell(&c.entry_cache.actual_cache_0108fe88) &&
        &c.entry_cache.actual_manager_01090aa0 == &c.generated.actual_manager_01090aa0,
        "records40 providers must share renderer, synchronization, entry-cache and manager cells");
    require(c.pass.actual_vertex_constants_0108ebf4 == c.pass.constants.bank0108EBF4 &&
        c.pass.actual_pixel_constants_0108dbec == c.pass.constants.bank0108DBEC &&
        cell(&c.pass.constants.actual_renderer_00f8d394) == cell(&c.pass.actual_renderer_00f8d394) &&
        cell(&c.cameras.viewport.renderer_00f8d394) == cell(&c.generated.actual_renderer_00f8d394) &&
        &c.cameras.viewport.one_bits_00d7a24c == &c.raw_models.constants.one_00d7a24c,
        "records40 requires the same raw constant banks and camera publication domain");
    require(c.mapping.actual_physical_lock.actual_lifetime_01090aa0.borrows_same_domain(
        c.generated.actual_manager_01090aa0) &&
        &c.pass.diagnostic_strings.actual_published_01090aa8 == &names.actual_published_01090aa8 &&
        &c.pass.diagnostic_strings.actual_small_returns_disabled_01090aa4 == &names.actual_small_returns_disabled_01090aa4 &&
        &c.pass.diagnostic_strings.actual_manager_publication_01090aa0 == &names.actual_manager_publication_01090aa0,
        "records40 mapping and diagnostic strings require the same actual raw manager/name cells");
    context_ = &c;
    phase_ = Phase::preparing;
    try {
        camera_scene_ = c.cameras.nodes.scenes.reserve_binding();
        camera_lifetime_ = c.cameras.nodes.attachments.reserve_binding();
        viewport_admission_ = c.viewports.admit(viewport_record_);
        phase_ = Phase::prepared;
    } catch (...) { cancel_preparation(); throw; }
}
void NativeRendererDebugRecords40Frame::settle_admissions() noexcept {
    camera_scene_.cancel(); camera_lifetime_.cancel(); viewport_admission_.cancel();
}
void NativeRendererDebugRecords40Frame::cancel_preparation() noexcept {
    if (phase_ != Phase::preparing && phase_ != Phase::prepared) std::terminate();
    settle_admissions();
    context_->viewports.forget_quiescent(viewport_record_);
    context_ = nullptr;
    phase_ = Phase::fresh;
}
void NativeRendererDebugRecords40Frame::retire_camera(void* opaque, NativeCameraReference& reference) noexcept {
    auto& f = *static_cast<NativeRendererDebugRecords40Frame*>(opaque);
    if (f.camera_reference != &reference || f.camera_retired_) std::terminate();
    if (f.camera_registered_) {
        const auto& registration = f.context_->generated.model_and_geometry.geometry.registration();
        registration.unbind(registration.context, f.camera_slot, reference);
        f.camera_registered_ = false;
    }
    f.camera_retired_ = true;
}
void NativeRendererDebugRecords40Frame::retire_after_host_quiescence() noexcept {
    if (phase_ != Phase::complete && phase_ != Phase::failed) std::terminate();
    if (camera_reference && !camera_retired_) std::terminate();
    if (camera_owner && camera_owner->phase != NativeCameraOwner::Phase::dead) std::terminate();
    if (viewport_record_.phase() == NativeViewportRegistry::Phase::live ||
        viewport_record_.phase() == NativeViewportRegistry::Phase::reserved) std::terminate();
    camera_reference_storage_.reset(); camera_owner_storage_.reset();
    context_->viewports.forget_quiescent(viewport_record_);
    phase_ = Phase::retired;
}
void NativeRendererDebugRecords40Frame::construct_camera() {
    auto& c = *context_;
    auto& pool = c.raw_models.names;
    reached_callsite = 0x00b2b92a;
    camera_slot = allocate_native_camera_slot_00b71930();
    int state = 3;
    bool name_live = false;
    try {
        if (camera_slot) {
            put(&camera_name_, 0, 0); put(&camera_name_, 4, 0);
            resize_native_string_header_0041dd40(&camera_name_, pool, 0x0e, true);
            if (void* data = read<void*>(&camera_name_, 4))
                std::memmove(data, "2dSpriteCamera", read<U>(&camera_name_) + 1u);
            state = 4; name_live = true;
            camera_owner_storage_.emplace(camera_slot, NativeCameraPool::slot_bytes,
                c.cameras, std::move(camera_scene_));
            camera_owner = &*camera_owner_storage_;
            reached_callsite = 0x00b2b98e;
            (void)construct_native_camera_00b71a80(*camera_owner, &camera_name_,
                c.raw_models.constants, std::move(viewport_admission_));
        }
        state = -1; // B2B99E: normal name return and all later work retain camera.
        if (name_live) destroy_native_string_header_0041dd20(&camera_name_, pool);
    } catch (...) {
        try {
            if (state == 4 && name_live) {
                name_live = false; state = 3;
                destroy_native_string_header_0041dd20(&camera_name_, pool);
            }
            if (state == 3) {
                state = -1;
                camera_slot_return_started = true;
                if (viewport_record_.phase() == NativeViewportRegistry::Phase::live)
                    retained_constructor_viewport = read<NativeViewportOwner*>(camera_slot, 0x180);
                camera_owner_storage_.reset(); camera_owner = nullptr;
                return_native_camera_slot_00b71350(camera_slot); // CBD488.
            }
        } catch (...) { std::terminate(); }
        throw;
    }
    if (camera_owner) {
        camera_reference_storage_.emplace(*camera_owner,
            NativeCameraCompanionDisposal{this, &NativeRendererDebugRecords40Frame::retire_camera},
            std::move(camera_lifetime_));
        camera_reference = &*camera_reference_storage_;
        const auto& registration = c.generated.model_and_geometry.geometry.registration();
        registration.bind(registration.context, camera_slot, *camera_reference);
        camera_registered_ = true;
    }
}

void __fastcall draw_native_renderer_debug_records40_00b2b580(void* renderer,
    NativeRendererDebugRecords40Context* context, NativeRendererDebugRecords40Frame* frame) {
    if (read<U>(renderer, 0x1d1c) == 0) return;
    require(context && frame && frame->context_ == context &&
        frame->phase_ == NativeRendererDebugRecords40Frame::Phase::prepared,
        "nonempty records40 needs its distinct prepared persistent frame");
    auto& c = *context;
    auto& f = *frame;
    f.phase_ = NativeRendererDebugRecords40Frame::Phase::running;
    try {
        if (read<U>(renderer, 0x19e8) == 0) {
            f.reached_callsite = 0x00b2b607;
            make_cached_model(renderer, c, f.cached_model);
        }
        const U count = read<U>(renderer, 0x1d1c);
        void* model = read<void*>(renderer, 0x19e8);
        f.captured_lock_vertices = count * 6u;
        void* current_stream = stream(model);
        stream_slot(current_stream, c, 0x10, 0x00b49980);
        f.locked_stream = current_stream;
        f.reached_callsite = 0x00b2b72d;
        f.locked_output = lock_native_logical_vertex_stream_00b49980(
            current_stream, c.mapping, f.captured_lock_vertices, 0, 0);
        write_native_debug_records40_vertices_00b2b741(renderer, f.locked_output, &c.vertices);
        current_stream = stream(read<void*>(renderer, 0x19e8));
        stream_slot(current_stream, c, 0x14, 0x00b49a80);
        f.reached_callsite = 0x00b2b8ec;
        unlock_native_logical_vertex_stream_00b49a80(current_stream, c.mapping);
        f.locked_stream = nullptr; f.locked_output = nullptr;
        model = read<void*>(renderer, 0x19e8);
        const U current_count = read<U>(renderer, 0x1d1c);
        put(section(model), 0x18, current_count * 2u);
        put(section(read<void*>(renderer, 0x19e8)), 0x10, f.captured_lock_vertices);
        f.construct_camera();
        // The original null allocation faults on the reached camera operation.
        // This source requires actual successful storage; no fallback is made.
        CameraMatrix matrix;
        identity(matrix, c.raw_models.constants.one_00d7a24c);
        f.reached_callsite = 0x00b2ba3c;
        require(f.camera_owner != nullptr, "records40 reached a null native camera allocation");
        set_native_camera_view_00b71490(*f.camera_owner, matrix.data());
        identity(matrix, c.raw_models.constants.one_00d7a24c);
        f.reached_callsite = 0x00b2bab3;
        set_camera_projection_00b6fd60(f.camera_owner->projection, matrix);
        f.reached_callsite = 0x00b2babc;
        gather_native_system_constants_00b46a70(nullptr, f.camera_slot, c.gather, f.gather);
        void* cache = c.entry_cache.actual_cache_0108fe88;
        f.captured_entry_cache = cache;
        f.entry_increment_started = true;
        const auto increment = c.actual_increment_00ce221c;
        const long used = increment(static_cast<volatile long*>(at(cache, 8)));
        float depth, visibility, leading;
        void* entry;
        // B2BAD4..BAEF: bank read between FLDZ and its spill; current model
        // read between FLD1 and its spill. Keep all four x87 rows and wrap.
        __asm {
            mov eax, used
            fldz
            mov ecx, cache
            mov edx, dword ptr [ecx + 4]
            fstp depth
            lea ecx, [eax + eax * 4]
            fld1
            lea edx, [edx + ecx * 8 - 28h]
            mov entry, edx
            mov ecx, renderer
            mov ecx, dword ptr [ecx + 19e8h]
            mov model, ecx
            fstp visibility
        }
        f.entry = entry;
        void* captured_geometry = geometry(model);
        void* current_model = read<void*>(renderer, 0x19e8);
        void* captured_section = section(current_model);
        __asm {
            fldz
            fstp leading
        }
        f.reached_callsite = 0x00b2bb18;
        initialize_native_render_entry_00b51a20(entry, &c.render_entry, leading,
            captured_section, captured_geometry, model, f.camera_slot, visibility, depth, 0);
        void* material = read<void*>(section(read<void*>(renderer, 0x19e8)), 0x20);
        void* effect = read<void*>(material, 0x7c);
        void* pass = read<void*>(effect, 0x9c);
        f.reached_callsite = 0x00b2bb43;
        execute_native_material_pass_current08(pass, entry, c.pass, f.pass);
        if (f.camera_slot) {
            f.camera_creator_release_started = true;
            f.reached_callsite = 0x00b2bb4b;
            unlink_and_release_render_model_00b6dfa0(*f.camera_reference);
        }
        void* header = at(renderer, 0x1d18);
        if (read<std::int32_t>(header, 8) < 0)
            reserve_native_renderer_records40_00b22a70(header, 0, 0);
        while (read<std::int32_t>(header, 4) > 0) put(header, 4, read<U>(header, 4) - 1u);
        put(header, 4, 0);
        f.settle_admissions();
        f.phase_ = NativeRendererDebugRecords40Frame::Phase::complete;
    } catch (...) {
        f.settle_admissions();
        f.phase_ = NativeRendererDebugRecords40Frame::Phase::failed;
        throw;
    }
}
} // namespace bsp
