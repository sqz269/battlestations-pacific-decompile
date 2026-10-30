#include "bsp/native_front_end_frame_lifetime.hpp"

#include "bsp/native_gui_page_registry.hpp"
#include "bsp/native_platform_focus_owners.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4);

Word address(const void* pointer) {
    return static_cast<Word>(reinterpret_cast<std::uintptr_t>(pointer));
}
void* pointer(Word identity) {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(identity));
}
Word load(Word actual_address) {
    return *reinterpret_cast<const volatile Word*>(
        static_cast<std::uintptr_t>(actual_address));
}
void store(Word actual_address, Word value) {
    *reinterpret_cast<volatile Word*>(
        static_cast<std::uintptr_t>(actual_address)) = value;
}

void retire_group(Word captured_frame, Word first_offset, Word columns,
    Word set_stride, const NativeFrontEndFrameLifetimeContext& c) {
    for (Word column = 0; column < columns; ++column) {
        for (Word set = 0; set < 5; ++set) {
            const Word slot = captured_frame + first_offset + column * 4u
                + set * set_stride;
            if (load(slot) == 0) continue;
            void* const manager = get_native_gui_manager_004c12b0(c.gui_owners);
            // Native517DEA/517E21/517E61 reload AFTER the whole getter.
            // Pass even a now-null slot unchanged; no post-getter null gate.
            void* const page = pointer(load(slot));
            remove_and_destroy_native_gui_page_00aa31f0(
                manager, page, c.page_lifetime);
        }
    }
}

// Native517E7F/517E85 and state1's existing517CF0(capturedFrame+8) schedule.
// Publication clear is unconditional, including a newer Frame generation.
void clear_publication_and_restore_secondary(Word captured_frame,
    const NativeFrontEndFrameLifetimeContext& c) {
    c.actual_frame_publication_00e18d80 = nullptr;
    store(captured_frame + 8u, 0x00ce3818u);
}

// Whole existing4F71A0 private composition, not another recovered-body credit.
// Stamp BEFORE the first bank read; clear all95 exact matches, never sentinel.
void destroy_screen_base_004f71a0(Word captured_frame,
    const NativeFrontEndFrameLifetimeContext& c) {
    store(captured_frame, 0x00ceae54u);
    for (Word index = 0; index < 95; ++index) {
        if (c.actual_screen_words_00e18b60[index] == captured_frame)
            c.actual_screen_words_00e18b60[index] = 0;
    }
}
} // namespace

__declspec(noinline) void destroy_native_front_end_frame_00517d90(
    void* captured_actual_frame, const NativeFrontEndFrameLifetimeContext& c) {
    const Word captured_frame = address(captured_actual_frame);
    store(captured_frame, 0x00cec39cu);
    store(captured_frame + 8u, 0x00cec398u);
    // Native state1 is armed AFTER both profile stores. Every reached getter
    // and page call in all85 slots is covered by the same base cleanup chain.
    try {
        retire_group(captured_frame, 0x0cu, 13, 0x34u, c);
        retire_group(captured_frame, 0x110u, 2, 8, c);
        retire_group(captured_frame, 0x138u, 2, 8, c);
    } catch (...) {
        // C6A838: state1->0, clear pub/restore captured secondary. C6A830:
        // state0->-1, whole4F71A0. No Page traversal or native effect is retried.
        clear_publication_and_restore_secondary(captured_frame, c);
        destroy_screen_base_004f71a0(captured_frame, c);
        throw;
    }
    clear_publication_and_restore_secondary(captured_frame, c);
    // Native517E8C disarms state before the normal base call at517E94.
    destroy_screen_base_004f71a0(captured_frame, c);
}

__declspec(noinline) void* delete_native_front_end_frame_00517eb0(
    void* actual_frame, volatile std::uint32_t& actual_flags_dword_home,
    const NativeFrontEndFrameLifetimeContext& c) {
    const Word captured_allocation = address(actual_frame);
    destroy_native_front_end_frame_00517d90(pointer(captured_allocation), c);
    // Native517EB8 consumes only BYTE[ESP+8], after the whole call returns.
    const unsigned char current_low_byte =
        *reinterpret_cast<const volatile unsigned char*>(&actual_flags_dword_home);
    if ((current_low_byte & 1u) != 0)
        singleton_lifetime_free(pointer(captured_allocation));
    return pointer(captured_allocation);
}

__declspec(noinline) void* delete_native_front_end_frame_secondary_00517d50(
    void* actual_secondary, volatile std::uint32_t& actual_flags_dword_home,
    const NativeFrontEndFrameLifetimeContext& c) {
    const Word captured_secondary = address(actual_secondary);
    const Word allocation = captured_secondary - 8u;
    return delete_native_front_end_frame_00517eb0(
        pointer(allocation), actual_flags_dword_home, c);
}
} // namespace bsp
