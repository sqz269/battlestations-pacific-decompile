#include "bsp/native_render_resource_init.hpp"
#include "bsp/native_renderer_current_depth_surface.hpp"
#include "bsp/native_renderer_surface_save_publish.hpp"
#include "bsp/platform_renderer_activation.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native render-resource initialization requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
void* at(const void* p, Word offset = 0) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<Word>(p) + offset);
}
Word word(const void* p, Word offset = 0) noexcept {
    return *static_cast<const volatile Word*>(at(p, offset));
}
void put(void* p, Word offset, Word value) noexcept {
    *static_cast<volatile Word*>(at(p, offset)) = value;
}
std::uint8_t byte(const void* p, Word offset) noexcept {
    return *static_cast<const volatile std::uint8_t*>(at(p, offset));
}
void put_byte(void* p, Word offset, std::uint8_t value) noexcept {
    *static_cast<volatile std::uint8_t*>(at(p, offset)) = value;
}
void* child(const void* p, Word offset) noexcept {
    return reinterpret_cast<void*>(word(p, offset));
}
void* renderer(NativeRenderResourceInitEntryContext& c, Word slot, Word expected) {
    void* const current = c.frame_targets.actual_surface_context.actual_renderer_00f8d394;
    if (!current || word(current) != 0x00d5f0a8u || !c.actual_renderer_profile_00d5f0a8 ||
        c.actual_renderer_profile_00d5f0a8[slot / 4u] != expected)
        throw std::logic_error("unsupported current render-resource renderer dispatch");
    return current;
}
void terminal_frame(void* captured, NativeRenderResourceInitEntryContext& c) {
    // B10868 captures current slot0; BD30E0 then refreshes the deleting slot.
    if (word(captured) != 0x00d5e600u || !c.actual_frame_profile_00d5e600 ||
        c.actual_frame_profile_00d5e600[0] != 0x00bd30e0u ||
        word(captured) != 0x00d5e600u || c.actual_frame_profile_00d5e600[1] != 0x00b1fcf0u)
        throw std::logic_error("unsupported current render-resource frame terminal");
    delete_native_frame_target_owner_00b1fcf0(
        *static_cast<NativeFrameTargetOwnerStorage*>(captured), 1, c.frame_targets);
}
Word remainder_eight(Word value) noexcept {
    Word result = value & 0x80000007u;
    if (result & 0x80000000u) result = ((result - 1u) | 0xfffffff8u) + 1u;
    return result;
}
} // namespace

void* copy_native_renderer_dimensions_00b21f10(const void* actual_renderer,
    void* actual_output) noexcept {
    const Word width = word(actual_renderer, 0x1a28);
    const Word height = word(actual_renderer, 0x1a2c);
    put(actual_output, 0, width);
    put(actual_output, 4, height);
    return actual_output;
}

void begin_native_render_resource_init_00b107f0_fragment(void* service,
    const volatile NativeRenderResourceInitArguments& arguments,
    NativeRenderResourceInitEntryContext& c, NativeRenderResourceInitEntryState& a) {
    if (a.phase != NativeRenderResourceInitEntryState::Phase::fresh)
        throw std::logic_error("render-resource entry continuation cannot be replayed");
    a.phase = NativeRenderResourceInitEntryState::Phase::running;
    a.service = service;
    a.argument_cells = &arguments;
    a.native_site = 0x00b1080b;
    try {
        // Native reads only AL, before the initial gate sample and +219 store.
        const auto incoming = *reinterpret_cast<const volatile std::uint8_t*>(&arguments.word_04);
        const bool initialized = byte(service, 0x1c4) != 0;
        put_byte(service, 0x219, incoming);
        if (initialized) {
            if (void* const batch = child(service, 0x20)) {
                a.native_site = 0x00b10832;
                mark_native_render_batch_dirty_00b50010(batch);
            }
            if (byte(service, 0x1c4) != 0) {
                if (void* const root = child(service, 0x30)) {
                    a.native_site = 0x00b1300b;
                    invalidate_native_render_root_chain_00b4ecc0(root);
                }
            }
            a.native_site = 0x00b13026;
            a.phase = NativeRenderResourceInitEntryState::Phase::already_initialized_returned;
            return;
        }

        void* const old = child(service, 0x1d4); // EDI, before current IAT read.
        a.captured_old_frame = old;
        if (old) {
            a.native_site = 0x00b1085e;
            const auto decrement = c.decrement_iat_00ce2220;
            if (!decrement) throw std::logic_error("render-resource decrement import is unbound");
            if (decrement(static_cast<volatile long*>(at(old, 4))) == 0) {
                a.native_site = 0x00b1086e;
                terminal_frame(old, c);
            }
            put(service, 0x1d4, 0); // AFTER callback, overwrites its parent-field edits.
        }
        a.native_site = 0x00b10878;
        void* const allocation = singleton_lifetime_allocate(
            {SingletonAllocationKind::object, 0x40, 0x40});
        a.frame_allocation = allocation;
        a.unwind_state = 0;
        void* frame = nullptr;
        if (allocation) {
            a.native_site = 0x00b10891;
            frame = construct_native_frame_target_owner_00b1fbb0(allocation);
        }
        put(service, 0x1d4, reinterpret_cast<Word>(frame));
        void* current = renderer(c, 0x128, 0x00b24dc0);
        a.unwind_state = -1;
        a.native_site = 0x00b108b9;
        void* surface = native_renderer_field197c_00b24dc0(current, nullptr, 0);
        frame = child(service, 0x1d4);
        a.native_site = 0x00b108c3;
        set_native_frame_target_color_00b1fab0(*static_cast<NativeFrameTargetOwnerStorage*>(frame),
            0, static_cast<NativeSurfaceOwnerStorage*>(surface), c.frame_targets);
        current = renderer(c, 0x12c, 0x00b20090);
        a.native_site = 0x00b108d6;
        surface = get_native_renderer_current_depth_surface_00b20090(current);
        frame = child(service, 0x1d4);
        a.native_site = 0x00b108df;
        set_native_frame_target_depth_00b1fb00(*static_cast<NativeFrameTargetOwnerStorage*>(frame),
            static_cast<NativeSurfaceOwnerStorage*>(surface), c.frame_targets);

        const Word first0 = word(service, 0x294), first1 = word(service, 0x290);
        const Word first2 = word(service, 0x28c), first3 = word(service, 0x298);
        put_byte(service, 0x1c4, 1);
        put_byte(service, 0x218, 1);
        put_byte(service, 0xb0, 1);
        put(service, 0x628, first0); put(service, 0x62c, first1);
        put(service, 0x630, first2); put(service, 0x634, first3);
        const Word second0 = word(service, 0x2a4), second1 = word(service, 0x2a0);
        const Word second2 = word(service, 0x29c), second3 = word(service, 0x2a8);
        put(service, 0x638, second0); put(service, 0x63c, second1);
        put(service, 0x640, second2); put(service, 0x644, second3);
        a.scalar_low_xmm[0] = second0; a.scalar_low_xmm[1] = second1;
        a.scalar_low_xmm[2] = second2; a.scalar_low_xmm[3] = second3;
        current = renderer(c, 0x80, 0x00b21f10);
        a.native_site = 0x00b1098c;
        copy_native_renderer_dimensions_00b21f10(current, a.dimensions_esp18);
        a.height_remainder_ecx = remainder_eight(a.dimensions_esp18[1]);
        a.aligned_height_esp24 = a.dimensions_esp18[1] - a.height_remainder_ecx;
        a.width_remainder_edx = remainder_eight(a.dimensions_esp18[0]);
        a.aligned_width_eax = a.dimensions_esp18[0] - a.width_remainder_edx;
        a.native_site = 0x00b109bc;
        a.phase = NativeRenderResourceInitEntryState::Phase::awaiting_b109bc_continuation;
    } catch (...) {
        a.phase = NativeRenderResourceInitEntryState::Phase::failed;
        // This bounded provider has no native FH3/SEH cleanup contract. A
        // diagnostic must retain current publication/attempt state for recovery.
        throw;
    }
}

} // namespace bsp
