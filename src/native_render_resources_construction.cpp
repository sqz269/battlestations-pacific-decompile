#include "bsp/native_render_resources_construction.hpp"
#include "bsp/native_render_service_vector_cleanup.hpp"
#include "bsp/native_renderer_current_depth_surface.hpp"
#include "bsp/native_renderer_surface_save_publish.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <exception>
#include <stdexcept>
#include <utility>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native render resources construction requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
void* at(void* base, Word offset = 0) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(base) + offset);
}
Word word(const void* p) noexcept { return *static_cast<const volatile Word*>(p); }
void put(void* p, Word value) noexcept { *static_cast<volatile Word*>(p) = value; }
void put_byte(void* p, unsigned char value) noexcept {
    *static_cast<volatile unsigned char*>(p) = value;
}
void* pointer(Word value) noexcept { return reinterpret_cast<void*>(value); }
Word pointer_word(const void* p) noexcept { return reinterpret_cast<std::uintptr_t>(p); }
void* allocate(Word bytes) {
    return singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes});
}

void unwind(NativeRenderResourcesConstructionContext& c,
    NativeRenderResourcesConstructionAcquired& a) {
    auto& strings = c.textures.strings;
    if (a.unwind_state >= 4) {
        const auto state = a.unwind_state;
        a.unwind_state = 3;
        if (state == 5 || state == 6) {
            destroy_native_string_header_0041dd20(a.native_locals_10_17, strings);
        } else {
            // CBC3C2/CBC3DD/CBC3E8 share the exact EBP-14 allocation spill.
            singleton_lifetime_free(pointer(word(a.native_locals_10_17)));
            if (state == 8) a.helper_allocation_freed = true;
        }
    }
    while (a.unwind_state >= 0) {
        const auto state = a.unwind_state--;
        switch (state) {
        case 3: destroy_native_string_header_vector_004324a0(at(a.owner, 0x69c), strings); break;
        case 2: destroy_native_renderer_record_vector_thunk_00b14590(at(a.owner, 0x68c), strings); break;
        case 1: destroy_native_string_header_0041dd20(at(a.owner, 0x684), strings); break;
        case 0: destroy_native_render_service_base_00b0f0c0(a.owner, c.base); break;
        }
    }
}

int cleanup_exception(unsigned long code) noexcept {
    if (code == 0xe06d7363u) std::terminate();
    return EXCEPTION_CONTINUE_SEARCH;
}
void unwind_during_exception(NativeRenderResourcesConstructionContext& c,
    NativeRenderResourcesConstructionAcquired& a) noexcept {
    // Terminate a second C++ exception during search, before nested cleanup.
    __try { unwind(c, a); }
    __except (cleanup_exception(GetExceptionCode())) { __assume(0); }
}

void* current_renderer(NativeRenderResourcesConstructionContext& c, Word slot, Word target) {
    void* const renderer = const_cast<void*>(c.textures.cache.textures.current_renderer_00f8d394);
    if (word(renderer) != 0x00d5f0a8u || !c.textures.renderer_profile ||
        c.textures.renderer_profile[slot / 4] != target)
        throw std::invalid_argument("unsupported current render-resources renderer slot");
    return renderer;
}

void load_default(unsigned index, const void* literal, Word resize_site, Word load_site,
    Word offset, NativeRenderResourcesConstructionContext& c,
    NativeRenderResourcesConstructionAcquired& a) {
    auto* const name = a.native_locals_10_17;
    put(name, 0);
    put(name + 4, 0);
    a.native_site = resize_site;
    resize_native_string_header_0041dd40(name, c.textures.strings, 9, true);
    void* const copied = pointer(word(name + 4));
    if (copied) std::memmove(copied, literal, word(name) + 1u);
    // Capture renderer and slot before the native state5/6 store.
    void* const renderer = current_renderer(c, 0x64, 0x00b319b0);
    a.unwind_state = index == 0 ? 5 : 6;
    a.native_site = load_site;
    void* const result = load_native_renderer_texture_00b319b0(
        renderer, name, 0, c.textures.cache, &a.default_loads[index]);
    put(at(a.owner, offset), pointer_word(result));
    void* const released = pointer(word(name + 4));
    a.unwind_state = 3;
    if (released) {
        const Word size = word(name) + 1u;
        auto& strings = c.textures.strings;
        auto* const pool = native_string_pool_get_or_create_00419cc0(
            strings.actual_published_01090aa8, strings.actual_manager_publication_01090aa0);
        return_native_string_pool_00bd1510(pool, released, size,
            strings.actual_small_returns_disabled_01090aa4);
    }
}

void validate(NativeRenderResourcesConstructionContext& c,
    NativeRenderResourcesConstructionAcquired& a) {
    if (a.phase != NativeRenderResourcesConstructionAcquired::Phase::fresh ||
        a.texture_construction.phase != NativeRenderServiceTextureConstructionAcquired::Phase::fresh)
        throw std::logic_error("render-resources construction cannot replay");
    for (const auto& load : a.default_loads)
        if (load.phase != NativeTextureCacheAcquired::Phase::not_started || load.wrapper_started)
            throw std::logic_error("render-resources default texture attempt already used");
    for (const auto& load : a.texture_construction.loads)
        if (load.phase != NativeTextureCacheAcquired::Phase::not_started || load.wrapper_started)
            throw std::logic_error("render-resources child texture attempt already used");
    if (c.frame_targets.actual_surface_context.actual_string_pool_00419cc0.actual_storage() !=
            &c.textures.cache.strings ||
        reinterpret_cast<const volatile void*>(
            &c.frame_targets.actual_surface_context.actual_renderer_00f8d394) !=
        reinterpret_cast<const volatile void*>(
            &c.textures.cache.textures.current_renderer_00f8d394))
        throw std::logic_error("render-resources requires one surface/texture renderer and string domain");
    auto& names = c.camera.nodes.require_raw_name_pool();
    auto& strings = c.textures.strings;
    if (&c.base.actual_manager_01090aa0 != &strings.actual_manager_publication_01090aa0 ||
        &names.actual_manager_publication_01090aa0 != &strings.actual_manager_publication_01090aa0 ||
        &names.actual_published_01090aa8 != &strings.actual_published_01090aa8 ||
        &names.actual_small_returns_disabled_01090aa4 != &strings.actual_small_returns_disabled_01090aa4)
        throw std::logic_error("render-resources requires one actual raw string and singleton domain");
    if (!c.actual_helper_table_00d61854 || !c.viewport_release.actual_two_word_table_00d5e5f8 ||
        !c.textures.renderer_profile || !c.actual_literal_00d5e474 ||
        !c.actual_literal_00d5e468 || !c.actual_literal_00d5e460)
        throw std::invalid_argument("render-resources requires actual table and literal bindings");
    if (&c.cells.actual_00d7a24c != &c.parameters.actual_00d7a24c ||
        &c.cells.actual_00d7a24c != &c.node_constants.one_00d7a24c ||
        &c.cells.actual_00ce54a0 != &c.parameters.actual_00ce54a0 ||
        &c.cells.actual_00ce3854 != &c.parameters.actual_00ce3854 ||
        &c.cells.actual_00ce3800 != &c.parameters.actual_00ce3800 ||
        &c.cells.actual_00ce3d34 != &c.parameters.actual_00ce3d34)
        throw std::logic_error("render-resources requires identical shared constant cell bindings");
}
} // namespace

NativeCockpitHelperOwner* NativeRenderResourcesConstructionAcquired::helper_owner() noexcept {
    return helper_owner_ ? &*helper_owner_ : nullptr;
}
NativeCockpitHelperReference* NativeRenderResourcesConstructionAcquired::helper_reference() noexcept {
    return helper_reference_ ? &*helper_reference_ : nullptr;
}
void NativeRenderResourcesConstructionAcquired::record_helper_retirement(void* context,
    NativeCockpitHelperReference& reference) noexcept {
    auto& a = *static_cast<NativeRenderResourcesConstructionAcquired*>(context);
    if (!a.helper_reference_ || &*a.helper_reference_ != &reference || a.helper_retired)
        std::terminate();
    a.helper_retired = true;
}
void NativeRenderResourcesConstructionAcquired::forget_retired_helper_after_host_quiescence() noexcept {
    if (!helper_retired || !helper_reference_ || !helper_owner_ ||
        helper_owner_->phase != NativeCockpitHelperOwner::Phase::dead) std::terminate();
    helper_reference_.reset();
    helper_owner_.reset();
    // The camera/viewport block has its own explicit quiescence check/reset.
}

void* construct_native_render_resources_00b14a10(void* receiver,
    NativeRenderResourcesConstructionContext& c, NativeRenderResourcesConstructionAcquired& a) {
    validate(c, a);
    a.phase = NativeRenderResourcesConstructionAcquired::Phase::preparing;
    auto admission = a.cockpit.admit(c.camera, c.node_constants, c.viewport_registry);
    a.owner = receiver;
    a.phase = NativeRenderResourcesConstructionAcquired::Phase::running;
    try {
        a.native_site = 0x00b14a31;
        publish_native_render_service_base_00b0f020(receiver, c.base);
        Word xmm0, xmm1, xmm2, xmm3;
        put(at(receiver, 0x0), 0xd5e480); // 00B14A36
        xmm0 = c.cells.actual_00ce54a0; // 00B14A3C
        xmm1 = 0; // 00B14A44
        xmm2 = c.cells.actual_00ce3804; // 00B14A47
        xmm3 = c.cells.actual_00ce4bc4; // 00B14A4F
        put(at(receiver, 0x10), 0); // 00B14A59
        put(at(receiver, 0x14), 0); // 00B14A5C
        put(at(receiver, 0x84), xmm0); // 00B14A5F
        xmm0 = c.cells.actual_00ce74f8; // 00B14A67
        put(at(receiver, 0x88), xmm0); // 00B14A6F
        xmm0 = c.cells.actual_00ce8198; // 00B14A77
        put(at(receiver, 0x8c), xmm0); // 00B14A7F
        xmm0 = c.cells.actual_00ce6650; // 00B14A87
        put(at(receiver, 0x90), xmm0); // 00B14A8F
        xmm0 = c.cells.actual_00d7a24c; // 00B14A97
        put(at(receiver, 0x9c), xmm2); // 00B14A9F
        xmm2 = c.cells.actual_00ce3854; // 00B14AA7
        put(at(receiver, 0x94), xmm0); // 00B14AAF
        put(at(receiver, 0x98), xmm1); // 00B14AB7
        put(at(receiver, 0xa0), xmm2); // 00B14ABF
        xmm2 = c.cells.actual_00ce38b8; // 00B14AC7
        put(at(receiver, 0xa4), xmm2); // 00B14ACF
        xmm2 = c.cells.actual_00ce3800; // 00B14AD7
        put(at(receiver, 0xac), xmm0); // 00B14ADF
        put_byte(at(receiver, 0xa8), 0); // 00B14AE7
        put(at(receiver, 0x230), xmm0); // 00B14AED
        put(at(receiver, 0x234), xmm2); // 00B14AF5
        xmm2 = c.cells.actual_00ce3d08; // 00B14AFD
        put(at(receiver, 0x23c), xmm3); // 00B14B05
        xmm3 = c.cells.actual_00d1f980; // 00B14B0D
        put(at(receiver, 0x238), xmm2); // 00B14B15
        put(at(receiver, 0x240), xmm2); // 00B14B1D
        put(at(receiver, 0x244), xmm1); // 00B14B25
        put(at(receiver, 0x248), xmm1); // 00B14B2D
        put(at(receiver, 0x24c), xmm1); // 00B14B35
        put(at(receiver, 0x250), xmm0); // 00B14B3D
        put(at(receiver, 0x254), xmm0); // 00B14B45
        put(at(receiver, 0x264), xmm1); // 00B14B4D
        put(at(receiver, 0x268), xmm0); // 00B14B55
        xmm0 = c.cells.actual_00ce3d34; // 00B14B5D
        put(at(receiver, 0x258), xmm3); // 00B14B65
        put(at(receiver, 0x25c), xmm2); // 00B14B6D
        put(at(receiver, 0x260), xmm2); // 00B14B75
        put(at(receiver, 0x294), xmm1); // 00B14B7D
        xmm1 = c.cells.actual_00ce89d0; // 00B14B85
        put(at(receiver, 0x28c), xmm0); // 00B14B8D
        xmm0 = c.cells.actual_00ce89cc; // 00B14B95
        put(at(receiver, 0x29c), xmm1); // 00B14B9D
        xmm1 = c.cells.actual_00ce89c8; // 00B14BA5
        put(at(receiver, 0x290), xmm0); // 00B14BAD
        xmm0 = c.cells.actual_00d7a238; // 00B14BB5
        put(at(receiver, 0x2a0), xmm1); // 00B14BBD
        xmm1 = c.cells.actual_00ce3d30; // 00B14BC5
        a.unwind_state = 0; // 00B14BD3
        put(at(receiver, 0x298), xmm0); // 00B14BD7
        put(at(receiver, 0x2a4), xmm1); // 00B14BDF
        put(at(receiver, 0x2a8), xmm0); // 00B14BE7
        initialize_native_render_service_parameters_00b0cd80(at(receiver, 0x2ac), c.parameters); // 00B14BEF
        xmm0 = c.cells.actual_00ce7628; // 00B14BF4
        put(at(receiver, 0x314), xmm0); // 00B14BFC
        xmm0 = c.cells.actual_00ce6a04; // 00B14C04
        put(at(receiver, 0x318), 0); // 00B14C0C
        put(at(receiver, 0x59c), xmm0); // 00B14C18
        put(at(receiver, 0x5a0), xmm0); // 00B14C20
        put(at(receiver, 0x5a4), xmm0); // 00B14C28
        put(at(receiver, 0x684), 0); // 00B14C30
        put(at(receiver, 0x688), 0); // 00B14C32
        put(at(receiver, 0x690), 0); // 00B14C35
        put(at(receiver, 0x694), 0); // 00B14C3B
        put(at(receiver, 0x698), 0); // 00B14C41
        put(at(receiver, 0x6a0), 0); // 00B14C47
        put(at(receiver, 0x6a4), 0); // 00B14C4D
        put(at(receiver, 0x6a8), 0); // 00B14C53
        xmm0 = 0; // 00B14C59
        put(at(receiver, 0x224), xmm0); // 00B14C5C
        xmm0 = c.cells.actual_00d7a24c; // 00B14C64
        a.unwind_state = 3; // 00B14C6E
        put(at(receiver, 0x50), 0); // 00B14C73
        put_byte(at(receiver, 0x218), 0); // 00B14C76
        put_byte(at(receiver, 0x219), 0); // 00B14C7C
        put_byte(at(receiver, 0x21a), 0); // 00B14C82
        put_byte(at(receiver, 0x21b), 0); // 00B14C88
        put_byte(at(receiver, 0x21c), 0); // 00B14C8E
        put(at(receiver, 0x220), 0x1); // 00B14C94
        put(at(receiver, 0x228), xmm0); // 00B14C9E
        put_byte(at(receiver, 0x680), 0); // 00B14CA6
        put_byte(at(receiver, 0x681), 0); // 00B14CAC
        put_byte(at(receiver, 0x1c4), 0); // 00B14CB2
        put_byte(at(receiver, 0xb0), 0); // 00B14CB8
        put(at(receiver, 0x44), 0); // 00B14CBE
        put(at(receiver, 0x48), 0); // 00B14CC1
        put(at(receiver, 0x18), 0); // 00B14CC4
        put(at(receiver, 0x20), 0); // 00B14CC7
        put(at(receiver, 0x24), 0); // 00B14CCA
        put(at(receiver, 0x28), 0); // 00B14CCD
        put(at(receiver, 0x38), 0); // 00B14CD0
        put(at(receiver, 0x3c), 0); // 00B14CD3
        put(at(receiver, 0x40), 0); // 00B14CD6
        put(at(receiver, 0x1cc), 0); // 00B14CD9
        put(at(receiver, 0x1d4), 0); // 00B14CDF
        put(at(receiver, 0x1c8), 0); // 00B14CE5
        put(at(receiver, 0x650), 0); // 00B14CEB
        put(at(receiver, 0x658), 0); // 00B14CF1
        put(at(receiver, 0x65c), 0); // 00B14CF7
        put(at(receiver, 0x664), 0); // 00B14CFD
        put(at(receiver, 0x4c), 0); // 00B14D03
        put(at(receiver, 0x54), 0); // 00B14D06
        put(at(receiver, 0x58), 0); // 00B14D09
        put(at(receiver, 0x5c), 0); // 00B14D0C
        put(at(receiver, 0x2c), 0); // 00B14D0F
        put(at(receiver, 0x1d0), 0); // 00B14D12
        put(at(receiver, 0x74), 0); // 00B14D18
        put(at(receiver, 0x80), 0); // 00B14D1B
        put(at(receiver, 0x654), 0); // 00B14D21
        put(at(receiver, 0x7c), 0); // 00B14D27
        put(at(receiver, 0x78), 0); // 00B14D2A
        put(at(receiver, 0x30), 0); // 00B14D2D
        put(at(receiver, 0x668), 0); // 00B14D30
        put(at(receiver, 0x67c), 0); // 00B14D36
        put(at(receiver, 0x66c), 0); // 00B14D3C
        put(at(receiver, 0x670), 0); // 00B14D42
        put(at(receiver, 0x674), 0); // 00B14D48
        put(at(receiver, 0x678), 0); // 00B14D4E
        put(at(receiver, 0x1c), 0); // 00B14D54
        put(at(receiver, 0x60), 0); // 00B14D57
        put(at(receiver, 0x64), 0); // 00B14D5A
        put(at(receiver, 0x6c), 0); // 00B14D5D
        put(at(receiver, 0x68), 0); // 00B14D60
        put(at(receiver, 0x660), 0); // 00B14D63
        put(at(receiver, 0xc), 0); // 00B14D69
        put(at(receiver, 0x1c0), 0); // 00B14D6C
        a.native_site = 0x00b14d72;
        void* allocation = allocate(0x40);
        put(a.native_locals_10_17, pointer_word(allocation));
        a.unwind_state = 4;
        void* frame = nullptr;
        if (allocation) {
            a.native_site = 0x00b14d89;
            frame = construct_native_frame_target_owner_00b1fbb0(allocation);
        }
        put(at(receiver, 0x1d4), pointer_word(frame));
        void* renderer = current_renderer(c, 0x128, 0x00b24dc0);
        a.unwind_state = 3;
        a.native_site = 0x00b14dac;
        void* surface = native_renderer_field197c_00b24dc0(renderer, nullptr, 0);
        frame = pointer(word(at(receiver, 0x1d4)));
        a.native_site = 0x00b14db6;
        set_native_frame_target_color_00b1fab0(*static_cast<NativeFrameTargetOwnerStorage*>(frame),
            0, static_cast<NativeSurfaceOwnerStorage*>(surface), c.frame_targets);
        renderer = current_renderer(c, 0x12c, 0x00b20090);
        a.native_site = 0x00b14dc9;
        surface = get_native_renderer_current_depth_surface_00b20090(renderer);
        frame = pointer(word(at(receiver, 0x1d4)));
        a.native_site = 0x00b14dd2;
        set_native_frame_target_depth_00b1fb00(*static_cast<NativeFrameTargetOwnerStorage*>(frame),
            static_cast<NativeSurfaceOwnerStorage*>(surface), c.frame_targets);
        load_default(0, c.actual_literal_00d5e474, 0x00b14de7, 0x00b14e20, 0x668, c, a);
        load_default(1, c.actual_literal_00d5e468, 0x00b14e5c, 0x00b14e95, 0x67c, c, a);
        a.native_site = 0x00b14ec6;
        allocation = allocate(0xcc);
        put(a.native_locals_10_17, pointer_word(allocation));
        a.unwind_state = 7;
        void* textures = nullptr;
        if (allocation) {
            a.native_site = 0x00b14edd;
            textures = construct_native_render_service_textures_00b52550(
                allocation, c.textures, a.texture_construction);
        }
        a.unwind_state = 3;
        put(at(receiver, 0x34), pointer_word(textures));
        a.native_site = 0x00b14ef3;
        resize_native_string_header_0041dd40(at(receiver, 0x684), c.textures.strings, 4, false);
        void* const copied = pointer(word(at(receiver, 0x688)));
        if (copied) {
            const Word count = word(at(receiver, 0x684)); // no +1 for this literal
            std::memmove(copied, c.actual_literal_00d5e460, count);
        }
        a.native_site = 0x00b14f12;
        allocation = allocate(0x24);
        put(a.native_locals_10_17, pointer_word(allocation));
        a.helper_allocation = allocation;
        a.unwind_state = 8;
        if (allocation) {
            a.native_site = 0x00b14f29;
            construct_native_cockpit_helper_00b3c800(allocation, 0x24,
                c.parameters.actual_00d7a2f0, c.cells.actual_00ce38b8,
                c.viewport_release, std::move(admission));
            a.helper_completed = true;
        }
    } catch (...) {
        a.phase = NativeRenderResourcesConstructionAcquired::Phase::failed;
        unwind_during_exception(c, a);
        throw;
    }
    // Only host bookkeeping can fail after B3C800 returns. The native routine
    // has no further calls. Preserve helper/block identity across this boundary;
    // never route a binding diagnostic through native raw-allocation state8.
    if (a.helper_completed) {
        try {
            a.helper_owner_.emplace(*static_cast<NativeCockpitHelperStorage*>(a.helper_allocation),
                c.camera.nodes, c.actual_helper_table_00d61854);
            a.helper_reference_.emplace(*a.helper_owner_, NativeCockpitHelperCompanionDisposal{
                &a, NativeRenderResourcesConstructionAcquired::record_helper_retirement});
        } catch (...) {
            a.phase = NativeRenderResourcesConstructionAcquired::Phase::helper_binding_failed;
            throw;
        }
        a.native_site = 0x00b14f2e;
        put(at(receiver, 0x0c), pointer_word(a.helper_allocation));
    } else {
        a.native_site = 0x00b14f49;
        put(at(receiver, 0x0c), 0);
    }
    a.unwind_state = -1;
    a.phase = NativeRenderResourcesConstructionAcquired::Phase::complete;
    return receiver;
}
} // namespace bsp
