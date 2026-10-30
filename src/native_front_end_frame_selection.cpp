#include "bsp/native_front_end_frame_selection.hpp"

#include "bsp/native_gui_page_registry.hpp"
#include "bsp/native_string.hpp"

#include <cstddef>
#include <cstring>
#include <exception>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
static_assert(sizeof(long) == 4);
static_assert(sizeof(NativeFrontEndFrameStringTemporary) == 8);
static_assert(alignof(NativeFrontEndFrameStringTemporary) == 4);

std::uintptr_t address(const volatile void* p) noexcept {
    return reinterpret_cast<std::uintptr_t>(p);
}
void* at(const volatile void* p, Word offset) noexcept {
    // Win32 DWORD arithmetic also preserves negative/raw index bit patterns.
    return reinterpret_cast<void*>(address(p) + offset);
}
Word load(const volatile void* p, Word offset = 0) noexcept {
    return *static_cast<const volatile Word*>(at(p, offset));
}
void store(void* p, Word offset, Word value) noexcept {
    *static_cast<volatile Word*>(at(p, offset)) = value;
}
void* pointer(Word value) noexcept {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(value));
}
Word word(void* value) noexcept {
    return static_cast<Word>(address(value));
}
Word c_string_length(Word captured_name) noexcept {
    auto cursor = static_cast<std::uintptr_t>(captured_name);
    const auto first_after_start = cursor + 1u;
    unsigned char current;
    do {
        current = *reinterpret_cast<const volatile unsigned char*>(cursor);
        cursor += 1u;
    } while (current != 0);
    return static_cast<Word>(cursor - first_after_start);
}
void unwind_header(NativeFrontEndFrameStringTemporary& temporary,
    NativeStringRawPoolContext& strings) {
    // Cleanup may throw; Original secondary FH3/SEH transport is unproved.
    destroy_native_string_header_0041dd20(&temporary, strings);
}

void load_named_slot(void* actual_slot, Word captured_name,
    std::uint8_t add_reference, NativeFrontEndFrameStringTemporary& temporary,
    const NativeFrontEndFrameSelectionContext& c) {
    store(&temporary, 0, 0);
    store(&temporary, 4, 0);
    const Word length = c_string_length(captured_name);
    resize_native_string_header_0041dd40(&temporary, c.strings, length, true);
    const Word current_data = load(&temporary, 4);
    if (current_data != 0) {
        const Word current_bytes = load(&temporary) + 1u;
        std::memcpy(pointer(current_data), pointer(captured_name), current_bytes);
    }

    // Native state/temporary flag is armed AFTER resize+copy, before GUI getter.
    // A failure in either earlier operation must not invent header cleanup.
    try {
        void* const manager = c.bindings.gui_manager_004c12b0();
        void* const page = c.bindings.load_page_00aa5840(
            manager, &temporary, 1, add_reference);
        store(actual_slot, 0, word(page));
    } catch (...) {
        unwind_header(temporary, c.strings);
        throw;
    }
    // Native sets state=-1 and clears its temporary flag BEFORE pool return.
    // An exception here propagates without repeating the just-entered release.
    destroy_native_string_header_0041dd20(&temporary, c.strings);
}

struct Group {
    Word frame_offset;
    Word columns;
    volatile Word* names;
    NativeFrontEndFrameStringTemporary* temporary;
};

void register_group(void* frame, const Group& group,
    const NativeFrontEndFrameSelectionContext& c) {
    for (Word column = 0; column < group.columns; ++column) {
        const Word outer_own = load(frame, 0x160);
        const Word name = load(group.names,
            (outer_own * group.columns + column) * 4u);
        for (Word set = 0; set < 5; ++set) {
            void* const slot = at(frame,
                group.frame_offset + (set * group.columns + column) * 4u);
            if (set == load(frame, 0x160) && name != 0) {
                load_named_slot(slot, name, 0, *group.temporary, c);
            } else {
                store(slot, 0, 0);
            }
        }
    }
}

void retire_group(void* frame, Word frame_offset, Word columns,
    const volatile Word& requested_home,
    const NativeFrontEndFrameSelectionContext& c) {
    for (Word column = 0; column < columns; ++column) {
        for (Word set = 0; set < 5; ++set) {
            // Reload the real argument home at EVERY row, before slot access.
            if (set == requested_home) continue;
            void* const slot = at(frame,
                frame_offset + (set * columns + column) * 4u);
            const Word tested_page = load(slot);
            if (tested_page == 0) continue;
            if (load(pointer(tested_page), 4) == 1) {
                void* const manager = c.bindings.gui_manager_004c12b0();
                // 5182BD/51832D/51839D reload SLOT AFTER the getter callback.
                void* const current_page = pointer(load(slot));
                remove_and_destroy_native_gui_page_00aa31f0(
                    manager, current_page, c.page_lifetime);
            } else {
                void* const captured_page = pointer(load(slot));
                // CALL [CE2220] at5182CF/51833F/5183AF. Read the actual
                // target cell here, never once at entry or in a context copy.
                const auto decrement = c.decrement_00ce2220;
                if (decrement(static_cast<volatile long*>(at(captured_page, 4))) == 0) {
                    const Word profile = load(captured_page);
                    const volatile Word* const table =
                        c.bindings.current_page_table(captured_page, profile);
                    const Word target = table[0];
                    c.bindings.call_page_virtual00(captured_page, target);
                }
            }
            // Returned retirement clears the actual SLOT, including replacement.
            // No read of the possibly retired Page occurs after the call.
            store(slot, 0, 0);
        }
    }
}

void acquire_group(void* frame, const Group& group,
    const volatile Word& requested_home,
    const NativeFrontEndFrameSelectionContext& c) {
    // Native request/home reload is once per group, not once per slot.
    const Word requested = requested_home;
    const Word row_bytes = requested * group.columns * 4u;
    for (Word column = 0; column < group.columns; ++column) {
        void* const slot = at(frame, group.frame_offset + row_bytes + column * 4u);
        // CMP [slot],0; MOV name,[namecell]; JNZ. Preserve BOTH actual loads
        // and their order, including current name reads for an existing Page.
        const Word existing = load(slot);
        const Word name = load(group.names, row_bytes + column * 4u);
        if (existing != 0) continue;
        if (name != 0) {
            load_named_slot(slot, name, 1, *group.temporary, c);
        } else {
            store(slot, 0, 0);
        }
    }
}
} // namespace

__declspec(noinline) void register_native_front_end_frame_00517ed0(
    void* captured_actual_frame, const NativeFrontEndFrameSelectionContext& c,
    NativeFrontEndFrameRegistrationScratch& scratch) {
    (void)c.bindings.register_screen_004f71d0(captured_actual_frame);
    store(captured_actual_frame, 0x160, 4);
    register_group(captured_actual_frame,
        {0x0c, 13, c.backdrop_name_words_00e08500, &scratch.backdrop_panel}, c);
    register_group(captured_actual_frame,
        {0x110, 2, c.panel_name_words_00e08604, &scratch.backdrop_panel}, c);
    register_group(captured_actual_frame,
        {0x138, 2, c.title_name_words_00e0862c, &scratch.title}, c);
}

__declspec(noinline) void select_native_front_end_frame_set_00518250(
    void* captured_actual_frame, const volatile Word& requested_dword_home,
    const volatile std::uint8_t& commit_byte_home,
    const NativeFrontEndFrameSelectionContext& c,
    NativeFrontEndFrameSelectionScratch& scratch) {
    const Word initial_request = requested_dword_home;
    if (initial_request == load(captured_actual_frame, 0x160)) return;
    if (commit_byte_home != 0) {
        retire_group(captured_actual_frame, 0x0c, 13, requested_dword_home, c);
        retire_group(captured_actual_frame, 0x110, 2, requested_dword_home, c);
        retire_group(captured_actual_frame, 0x138, 2, requested_dword_home, c);
    }
    acquire_group(captured_actual_frame,
        {0x0c, 13, c.backdrop_name_words_00e08500, &scratch.backdrop},
        requested_dword_home, c);
    acquire_group(captured_actual_frame,
        {0x110, 2, c.panel_name_words_00e08604, &scratch.panel},
        requested_dword_home, c);
    acquire_group(captured_actual_frame,
        {0x138, 2, c.title_name_words_00e0862c, &scratch.title},
        requested_dword_home, c);
    if (commit_byte_home != 0) {
        const Word final_request = requested_dword_home;
        store(captured_actual_frame, 0x160, final_request);
    }
}

} // namespace bsp
