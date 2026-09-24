#include "bsp/native_gui_media_focus_lifetime.hpp"
#include "bsp/native_platform_focus_owners.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/singleton_lifetime.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <cstdlib>
#include <exception>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
void* at(void* p, Word offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<Word>(p) + offset);
}
volatile Word& word(void* p, Word offset = 0) noexcept {
    return *static_cast<volatile Word*>(at(p, offset));
}
void* pointer(void* p, Word offset = 0) noexcept {
    return reinterpret_cast<void*>(word(p, offset));
}
int cleanup_exception(unsigned long code) noexcept {
    if (code == 0xe06d7363u) std::terminate();
    return EXCEPTION_CONTINUE_SEARCH;
}
void unwind_gui(void* owner, NativeGuiMediaFocusLifetimeContext& context,
    int state) noexcept {
    // DEDB5C: 4->3->2->1->0->-1, funclets CB7329/1E/13/08/00.
    // +28 has no member-unwind action in this map.
    __try {
        if (state >= 4) destroy_native_gui_string_list_00aa5b20(at(owner, 0x34), context.strings);
        if (state >= 3) release_native_gui_resource_slot_0041da80(at(owner, 0x2c), context.owners.actual_resources);
        if (state >= 2) destroy_native_gui_page_vector_004c8020(at(owner, 0x14));
        if (state >= 1) destroy_native_gui_tree_00aa5260(at(owner, 8));
        if (state >= 0) destroy_native_gui_base_00aa0ff0(owner, context.owners);
    } __except (cleanup_exception(GetExceptionCode())) { __assume(0); }
}
struct GuiCleanup {
    void* owner;
    NativeGuiMediaFocusLifetimeContext& context;
    int state = 4;
    ~GuiCleanup() noexcept { if (state >= 0) unwind_gui(owner, context, state); }
};
}

void destroy_native_media_list_00a4c510(void* header) noexcept {
    void* const initial_head = pointer(header, 4);
    void* cursor = pointer(initial_head);
    word(initial_head) = reinterpret_cast<Word>(initial_head);
    void* const previous_head = pointer(header, 4);
    word(previous_head, 4) = reinterpret_cast<Word>(previous_head);
    const bool has_nodes = cursor != pointer(header, 4);
    word(header, 8) = 0;
    if (has_nodes) {
        do {
            void* const next = pointer(cursor);
            singleton_lifetime_free(cursor);
            cursor = next;
        } while (cursor != pointer(header, 4));
    }
    singleton_lifetime_free(pointer(header, 4));
    word(header, 4) = 0;
}

void destroy_native_media_manager_00a4c600(void* owner,
    NativePlatformFocusOwnersContext& context) noexcept {
    destroy_native_media_list_00a4c510(at(owner, 4));
    destroy_native_media_base_00a4c3f0(owner, context);
}

void* scalar_delete_native_media_manager_00a4c620(void* owner,
    Word flags, NativePlatformFocusOwnersContext& context) noexcept {
    destroy_native_media_manager_00a4c600(owner, context);
    if (flags & 1u) singleton_lifetime_free(owner);
    return owner;
}

void clear_native_gui_string_list_00aa51d0(void* header,
    NativeStringRawPoolContext& strings) {
    void* const initial_head = pointer(header, 4);
    void* cursor = pointer(initial_head);
    word(initial_head) = reinterpret_cast<Word>(initial_head);
    void* const previous_head = pointer(header, 4);
    word(previous_head, 4) = reinterpret_cast<Word>(previous_head);
    const bool has_nodes = cursor != pointer(header, 4);
    word(header, 8) = 0;
    if (has_nodes) {
        do {
            // Native captures data, then next, then length. The getter is
            // still called for large returns and disabled small returns.
            char* const data = static_cast<char*>(pointer(cursor, 0x0c));
            void* const next = pointer(cursor);
            if (data) {
                const Word size = word(cursor, 8) + 1u;
                auto* const pool = native_string_pool_get_or_create_00419cc0(
                    strings.actual_published_01090aa8,
                    strings.actual_manager_publication_01090aa0);
                return_native_string_pool_00bd1510(pool, data, size,
                    strings.actual_small_returns_disabled_01090aa4);
            }
            singleton_lifetime_free(cursor);
            cursor = next;
        } while (cursor != pointer(header, 4));
    }
}

void destroy_native_gui_string_list_00aa5b20(void* header,
    NativeStringRawPoolContext& strings) {
    clear_native_gui_string_list_00aa51d0(header, strings);
    singleton_lifetime_free(pointer(header, 4));
    word(header, 4) = 0;
}

void destroy_native_gui_manager_00aa6340_fragment(void* owner,
    NativeGuiMediaFocusLifetimeContext& context) {
    word(owner) = 0x00d5bfcc;
    Word cursor = word(owner, 0x18);
    const bool invalid_initial = cursor > word(owner, 0x1c);
    void* const vector = at(owner, 0x14);
    GuiCleanup cleanup{owner, context};
    if (invalid_initial) _invalid_parameter_noinfo();
    for (;;) {
        const Word end = word(vector, 8);
        if (word(vector, 4) > end) _invalid_parameter_noinfo();
        // CMP ESI,ESI makes AA6392 unreachable; preserve the captured end.
        if (cursor == end) break;
        if (cursor >= word(vector, 8)) _invalid_parameter_noinfo();
        context.pages.call_current_page_virtual_20(pointer(reinterpret_cast<void*>(cursor)));
        if (cursor >= word(vector, 8)) _invalid_parameter_noinfo();
        if (void* page = pointer(reinterpret_cast<void*>(cursor))) {
            context.pages.call_current_page_virtual_04(page, 1);
        }
        if (cursor >= word(vector, 8)) _invalid_parameter_noinfo();
        cursor += 4;
    }
    release_native_gui_resource_slot_0041da80(at(owner, 0x28), context.owners.actual_resources);
    cleanup.state = 3;
    destroy_native_gui_string_list_00aa5b20(at(owner, 0x34), context.strings);
    cleanup.state = 2;
    release_native_gui_resource_slot_0041da80(at(owner, 0x2c), context.owners.actual_resources);
    destroy_native_gui_page_vector_004c8020(vector);
    cleanup.state = 0;
    destroy_native_gui_tree_00aa5260(at(owner, 8));
    destroy_native_gui_base_00aa0ff0(owner, context.owners);
    cleanup.state = -1;
}

void* scalar_delete_native_gui_manager_00aa6540_fragment(void* owner,
    Word flags, NativeGuiMediaFocusLifetimeContext& context) {
    destroy_native_gui_manager_00aa6340_fragment(owner, context);
    if (flags & 1u) singleton_lifetime_free(owner);
    return owner;
}
} // namespace bsp
