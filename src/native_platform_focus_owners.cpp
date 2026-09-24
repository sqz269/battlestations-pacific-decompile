#include "bsp/native_platform_focus_owners.hpp"
#include "bsp/native_diagnostic_sink_lifetime.hpp"
#include "bsp/native_render_context.hpp"
#include "bsp/native_singleton_publication.hpp"
#include "bsp/native_singleton_vector_registration_wrappers.hpp"
#include "bsp/singleton_lifetime.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <cstddef>
#include <exception>

namespace bsp {
namespace {
using Word = std::uint32_t;
void* at(void* p, Word offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<Word>(p) + offset);
}
volatile Word& word(void* p, Word offset = 0) noexcept {
    return *static_cast<volatile Word*>(at(p, offset));
}
void* pointer(void* p, Word offset = 0) noexcept {
    return reinterpret_cast<void*>(word(p, offset));
}
void* allocate_list(Word bytes) {
    void* const allocation = singleton_lifetime_allocate(
        {SingletonAllocationKind::object, bytes, bytes});
    __asm {
        mov eax, allocation
        test eax, eax
        jz previous_link
        mov [eax], eax
    previous_link:
        lea ecx, [eax + 4]
        test ecx, ecx
        jz links_done
        mov [ecx], eax
    links_done:
    }
    return allocation;
}
struct NativeGuard { Word profile; CRITICAL_SECTION* section; };
static_assert(sizeof(void*) == 4 && sizeof(NativeGuard) == 8);
static_assert(offsetof(NativeGuard, section) == 4 && sizeof(CRITICAL_SECTION) == 0x18);
int cleanup_exception(unsigned long code) noexcept {
    if (code == 0xe06d7363u) std::terminate();
    return EXCEPTION_CONTINUE_SEARCH;
}
void unwind_guard(NativeGuard& guard) noexcept {
    __try { destroy_native_singleton_guard_00411ee0(&guard); }
    __except (cleanup_exception(GetExceptionCode())) { __assume(0); }
}
void unwind_gui_members(void* owner, NativePlatformFocusOwnersContext& context) noexcept {
    __try {
        release_native_gui_resource_slot_0041da80(at(owner, 0x2c), context.actual_resources);
        destroy_native_gui_page_vector_004c8020(at(owner, 0x14));
        destroy_native_gui_tree_00aa5260(at(owner, 8));
    } __except (cleanup_exception(GetExceptionCode())) { __assume(0); }
}
struct GuardCleanup {
    NativeGuard& guard;
    bool armed = true;
    ~GuardCleanup() noexcept { if (armed) unwind_guard(guard); }
};
struct AllocationCleanup {
    void* allocation;
    bool armed = true;
    ~AllocationCleanup() noexcept { if (armed) singleton_lifetime_free(allocation); }
};
struct BaseCleanup {
    void* owner;
    NativePlatformFocusOwnersContext& context;
    bool gui;
    bool armed = true;
    ~BaseCleanup() noexcept {
        if (armed) {
            if (gui) destroy_native_gui_base_00aa0ff0(owner, context);
            else destroy_native_media_base_00a4c3f0(owner, context);
        }
    }
};
struct GuiMembersCleanup {
    void* owner;
    NativePlatformFocusOwnersContext& context;
    bool armed = true;
    ~GuiMembersCleanup() noexcept { if (armed) unwind_gui_members(owner, context); }
};

template<bool Gui>
void* get_owner(NativePlatformFocusOwnersContext& context) {
    void* volatile& published = Gui ? context.actual_gui_00f8bc5c : context.actual_media_00f8aef8;
    void* const first = published;
    if (first) return first;
    void* const first_manager = get_native_singleton_manager_00415350(context.actual_manager_01090aa0);
    auto* const section = reinterpret_cast<CRITICAL_SECTION*>(word(first_manager, 0x10));
    NativeGuard guard{0x00ce37fcu, section};
    if (section) {
        EnterCriticalSection(section);
        word(section, 0x18) = word(section, 0x18) + 1u;
    }
    GuardCleanup guard_cleanup{guard}; // Native state0 only after Enter/depth.
    if (!published) {
        constexpr Word bytes = Gui ? 0x88u : 0x18u;
        void* const allocation = singleton_lifetime_allocate(
            {SingletonAllocationKind::object, bytes, bytes});
        AllocationCleanup owner_cleanup{allocation};
        void* constructed = nullptr;
        if (allocation) {
            if constexpr (Gui) constructed = construct_native_gui_manager_00aa5d70(allocation, context);
            else constructed = construct_native_media_manager_00a4c5a0(allocation, context);
        }
        owner_cleanup.armed = false; // state0 BEFORE publication store.
        published = constructed;
        void* const manager = get_native_singleton_manager_00415350(context.actual_manager_01090aa0);
        void* const current = published;
        register_native_singleton_object_00bd0c30(manager, nullptr, current);
    }
    if (section) {
        word(section, 0x18) = word(section, 0x18) - 1u;
        LeaveCriticalSection(section); // state0 remains armed through Leave.
    }
    void* const result = published;
    guard_cleanup.armed = false;
    return result;
}
} // namespace

void* allocate_native_gui_list_sentinel_00aa2820() { return allocate_list(0x14); }
void* allocate_native_media_list_sentinel_00a4c4a0() { return allocate_list(0x0c); }

void* allocate_native_gui_tree_node_00aa2920() {
    void* const allocation = singleton_lifetime_allocate(
        {SingletonAllocationKind::object, 0x18, 0x18});
    __asm {
        mov eax, allocation
        test eax, eax
        jz second_link
        mov dword ptr [eax], 0
    second_link:
        lea ecx, [eax + 4]
        test ecx, ecx
        jz third_link
        mov dword ptr [ecx], 0
    third_link:
        lea ecx, [eax + 8]
        test ecx, ecx
        jz node_flags
        mov dword ptr [ecx], 0
    node_flags:
        mov byte ptr [eax + 14h], 1
        mov byte ptr [eax + 15h], 0
    }
    return allocation;
}

void destroy_native_gui_base_00aa0ff0(void* owner, NativePlatformFocusOwnersContext& context) noexcept {
    context.actual_gui_00f8bc5c = nullptr;
    word(owner) = 0x00ce3818u;
}
void destroy_native_media_base_00a4c3f0(void* owner, NativePlatformFocusOwnersContext& context) noexcept {
    context.actual_media_00f8aef8 = nullptr;
    word(owner) = 0x00ce3818u;
}

void release_native_gui_resource_slot_0041da80(void* actual_slot,
    NativeRenderActualOwners& actual_resources) {
    void* const captured = pointer(actual_slot);
    if (captured) {
        release_native_render_actual_owner(actual_resources, captured);
        word(actual_slot) = 0;
    }
}

void destroy_native_gui_page_vector_004c8020(void* actual_header) noexcept {
    void* const captured = pointer(actual_header, 4);
    if (captured) singleton_lifetime_free(captured);
    word(actual_header, 4) = 0;
    word(actual_header, 8) = 0;
    word(actual_header, 0x0c) = 0;
}

void erase_native_gui_tree_subtree_00aa22c0(void* actual_tree, void* actual_node) {
    void* current = actual_node;
    while (*static_cast<volatile unsigned char*>(at(current, 0x15)) == 0) {
        erase_native_gui_tree_subtree_00aa22c0(actual_tree, pointer(current, 8));
        void* const left = pointer(current);
        singleton_lifetime_free(current);
        current = left;
    }
}

void destroy_native_gui_tree_00aa5260(void* actual_tree) {
    // AA5260 supplies (tree,*head.left,tree,head) toAA49E0. These are its
    // whole-range arguments for valid nonconcurrent storage. Preserve original
    // raw reads/stores of that branch; no general iterator interface is added.
    void* const initial_head = pointer(actual_tree, 4);
    const Word initial_first = word(initial_head);
    (void)initial_first;
    void* head = pointer(actual_tree, 4);
    const Word first = word(head);
    (void)first;
    head = pointer(actual_tree, 4);
    head = pointer(actual_tree, 4);
    erase_native_gui_tree_subtree_00aa22c0(actual_tree, pointer(head, 4));
    head = pointer(actual_tree, 4);
    word(head, 4) = reinterpret_cast<Word>(head);
    head = pointer(actual_tree, 4);
    word(actual_tree, 8) = 0;
    word(head) = reinterpret_cast<Word>(head);
    head = pointer(actual_tree, 4);
    word(head, 8) = reinterpret_cast<Word>(head);
    head = pointer(actual_tree, 4);
    const Word returned_first = word(head);
    (void)returned_first; // The native output iterator is private scratch.
    singleton_lifetime_free(pointer(actual_tree, 4));
    word(actual_tree, 4) = 0;
    word(actual_tree, 8) = 0;
}

void* construct_native_gui_manager_00aa5d70(void* owner, NativePlatformFocusOwnersContext& context) {
    BaseCleanup base{owner, context, true};
    word(owner) = 0x00d5bfccu;
    void* const allocated_head = allocate_native_gui_tree_node_00aa2920();
    word(owner, 0x0c) = reinterpret_cast<Word>(allocated_head);
    *static_cast<volatile unsigned char*>(at(allocated_head, 0x15)) = 1;
    void* head = pointer(owner, 0x0c);
    word(head, 4) = reinterpret_cast<Word>(head);
    head = pointer(owner, 0x0c);
    word(head) = reinterpret_cast<Word>(head);
    head = pointer(owner, 0x0c);
    word(head, 8) = reinterpret_cast<Word>(head);
    word(owner, 0x10) = 0;
    word(owner, 0x18) = 0;
    word(owner, 0x1c) = 0;
    word(owner, 0x20) = 0;
    word(owner, 0x24) = 0;
    word(owner, 0x28) = 0;
    word(owner, 0x2c) = 0;
    GuiMembersCleanup members{owner, context}; // Native state3 before+30 store.
    word(owner, 0x30) = 0;
    void* const list = allocate_native_gui_list_sentinel_00aa2820();
    const Word half = context.actual_half_00ce3800;
    word(owner, 0x38) = reinterpret_cast<Word>(list);
    word(owner, 0x3c) = 0;
    word(owner, 0x40) = 0;
    *static_cast<volatile unsigned char*>(at(owner, 0x48)) = 0;
    word(owner, 0x5c) = half;
    word(owner, 0x60) = half;
    word(owner, 0x6c) = 0;
    members.armed = false;
    base.armed = false;
    return owner;
}

void* construct_native_media_manager_00a4c5a0(void* owner, NativePlatformFocusOwnersContext& context) {
    BaseCleanup base{owner, context, false};
    word(owner) = 0x00d24d94u;
    void* const list = allocate_native_media_list_sentinel_00a4c4a0();
    const Word one = context.actual_one_00d7a24c;
    word(owner, 8) = reinterpret_cast<Word>(list);
    word(owner, 0x0c) = 0;
    word(owner, 0x10) = one;
    base.armed = false;
    return owner;
}

void* get_native_gui_manager_004c12b0(NativePlatformFocusOwnersContext& context) {
    return get_owner<true>(context);
}
void* get_native_media_manager_004c1710(NativePlatformFocusOwnersContext& context) {
    return get_owner<false>(context);
}
__declspec(naked) void __stdcall ignore_native_media_pause_00a4c2d0(std::uint32_t) {
    __asm { ret 4 }
}
} // namespace bsp
