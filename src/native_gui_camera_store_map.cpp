#include "bsp/native_gui_camera_store_map.hpp"
#include "bsp/native_int_pointer_tree18_leaves.hpp"
#include "bsp/gui_text_type_dispatch.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstdlib>

namespace bsp {
namespace {
using Word = std::uint32_t;
void* at(void* p, std::size_t offset) noexcept {
    return static_cast<unsigned char*>(p) + offset;
}
void* pointer(const void* p, std::size_t offset = 0) noexcept {
    return *reinterpret_cast<void* const volatile*>(
        static_cast<const unsigned char*>(p) + offset);
}
void put_pointer(void* p, std::size_t offset, void* value) noexcept {
    *reinterpret_cast<void* volatile*>(at(p, offset)) = value;
}
volatile Word& word(void* p, std::size_t offset = 0) noexcept {
    return *reinterpret_cast<volatile Word*>(at(p, offset));
}
void validate(bool value) { if (!value) _invalid_parameter_noinfo(); }
bool matches(const void* stored, const void* wanted) noexcept {
    bool result;
    __asm {
        mov ecx, stored
        mov edx, wanted
        mov eax, dword ptr [ecx]
        cmp eax, dword ptr [edx]
        jne mismatch
        fld dword ptr [edx + 4]
        fld dword ptr [ecx + 4]
        fucomip st, st(1)
        fstp st(0)
        lahf
        test ah, 44h
        jp mismatch
        fld dword ptr [edx + 8]
        fld dword ptr [ecx + 8]
        fucomip st, st(1)
        fstp st(0)
        lahf
        test ah, 44h
        jp mismatch
        fld dword ptr [edx + 0ch]
        fld dword ptr [ecx + 0ch]
        fucomip st, st(1)
        fstp st(0)
        lahf
        test ah, 44h
        jp mismatch
        fld dword ptr [edx + 14h]
        fld dword ptr [ecx + 14h]
        fucomip st, st(1)
        fstp st(0)
        lahf
        test ah, 44h
        jp mismatch
        mov result, 1
        jmp finished
    mismatch:
        mov result, 0
    finished:
    }
    return result;
}
const char* camera_name_data(void* camera, const char* empty) noexcept {
    const auto& name = native_node_name_00b6d800(
        *static_cast<const NativeNodeStorage*>(camera));
    const auto* const data = static_cast<const char*>(pointer(&name, 4));
    return data ? data : empty;
}
} // namespace

void __fastcall advance_native_gui_camera_store_iterator_004bda70(void* iterator) {
    increment_native_int_pointer_tree18_00869a20(iterator);
}
void __fastcall rotate_native_gui_camera_store_right_00aa1080(
    void* tree, void* unused_edx, void* pivot) {
    rotate_right_native_int_pointer_tree18_00869810(tree, unused_edx, pivot);
}
void __fastcall rotate_native_gui_camera_store_left_00aa2180(
    void* tree, void* unused_edx, void* pivot) {
    rotate_left_native_int_pointer_tree18_0086a2f0(tree, unused_edx, pivot);
}
void* allocate_native_gui_camera_store_tree_node_00aa2960(
    void* left, void* parent, void* right,
    const NativeGuiCameraStorePair* pair, std::uint8_t color) {
    return allocate_native_int_pointer_tree18_node_0086ab70(left, parent, right, pair, color);
}
NativeIntPointerTree18Iterator* link_native_gui_camera_store_tree_node_00aa41b0(
    void* tree, NativeIntPointerTree18Iterator* output, std::uint8_t insert_left,
    void* parent, const NativeGuiCameraStorePair* pair) {
    return link_native_int_pointer_tree18_node_0086ebe0(tree, output, insert_left, parent, pair);
}

NativeGuiCameraStoreInsertResult* insert_native_gui_camera_store_00aa4960(
    void* tree, NativeGuiCameraStoreInsertFrame& frame) {
    auto* const frame_address = &frame;
    const NativeGuiCameraStorePair* captured_pair;
    void* selected_parent;
    __asm {
        mov ecx, tree
        mov eax, dword ptr [ecx + 4]
        mov edx, dword ptr [eax + 4]
        cmp byte ptr [edx + 15h], 0
        mov esi, frame_address
        mov edi, dword ptr [esi + 10h]
        mov captured_pair, edi
        mov bl, 1
        mov byte ptr [esi], bl
        jne search_done
        movss xmm0, dword ptr [edi]
        movss dword ptr [esi + 10h], xmm0
        fld dword ptr [esi + 10h]
    descend:
        fld dword ptr [edx + 0ch]
        mov eax, edx
        fcomip st, st(1)
        jbe go_right
        mov edx, dword ptr [edx]
        mov byte ptr [esi], bl
        jmp next_node
    go_right:
        mov edx, dword ptr [edx + 8]
        mov byte ptr [esi], 0
    next_node:
        cmp byte ptr [edx + 15h], 0
        je descend
        fstp st(0)
    search_done:
        mov selected_parent, eax
    }
    const Word direction_word = word(&frame.local_00);
    auto* const result = link_native_gui_camera_store_tree_node_00aa41b0(
        tree, &frame.local_00, static_cast<std::uint8_t>(direction_word),
        selected_parent, captured_pair);
    auto* const output = frame.output_argument_0c;
    void* const owner = pointer(result);
    void* const node = pointer(result, 4);
    *static_cast<volatile unsigned char*>(at(output, 8)) = 1;
    put_pointer(output, 0, owner);
    put_pointer(output, 4, node);
    return output;
}

void* find_native_gui_camera_store_00aa3280(void* manager,
    const void* descriptor, NativeGuiCameraStoreFindScratch& scratch,
    const char* empty) {
    void* tree = at(manager, 8);
    void* cursor = pointer(pointer(tree, 4));
    void* owner = tree;
    put_pointer(&scratch, 0, tree);
    put_pointer(&scratch, 8, cursor);
    put_pointer(&scratch, 4, owner);
    for (;;) {
        void* const end = pointer(tree, 4);
        validate(owner != nullptr && owner == tree);
        if (cursor == end) return nullptr;
        validate(owner != nullptr);
        validate(cursor != pointer(owner, 4));
        void* const store = pointer(cursor, 0x10);
        if (matches(store, descriptor)) break;
        advance_native_gui_camera_store_iterator_004bda70(&scratch.iterator_04);
        cursor = pointer(&scratch, 8);
        owner = pointer(&scratch, 4);
        tree = pointer(&scratch);
    }
    validate(cursor != pointer(owner, 4));
    void* const store = pointer(cursor, 0x10);
    const char* const name = camera_name_data(pointer(store, 0x18), empty);
    validate(cursor != pointer(owner, 4));
    void* const diagnostic_store = pointer(cursor, 0x10);
    // 4254B0 is a verified single RET. Keep its original argument reads.
    (void)name;
    (void)diagnostic_store;
    validate(cursor != pointer(owner, 4));
    return pointer(cursor, 0x10);
}

void* create_native_gui_camera_store_00aa5070(void* manager,
    NativeGuiCameraStoreCreateFrame& frame, NativeGuiCameraStoreMapBindings& bindings,
    NativeGuiCameraStoreAcquired& acquired) {
    void* const store = singleton_lifetime_allocate(
        {SingletonAllocationKind::object, 0x24, 0x24});
    acquired.allocated_store = store;
    acquired.inserted = false;
    if (store) {
        const Word near_bits = bindings.actual_near_00d7a2f0;
        word(store) = 0;
        word(store, 4) = near_bits;
        const Word far_bits = bindings.actual_far_00ce3804;
        word(store, 8) = far_bits;
        const Word one_bits = bindings.actual_one_00d7a24c;
        word(store, 0x0c) = one_bits;
        word(store, 0x10) = 0;
        word(store, 0x14) = 0;
    }
    auto* const frame_address = &frame;
    __asm {
        mov edi, frame_address
        mov esi, store
        mov eax, dword ptr [edi + 3ch]
        mov ecx, dword ptr [eax]
        mov dword ptr [esi], ecx
        fld dword ptr [eax + 4]
        mov ecx, dword ptr [edi + 38h]
        fstp dword ptr [esi + 4]
        fld dword ptr [eax + 8]
        mov dword ptr [edi + 20h], esi
        fstp dword ptr [esi + 8]
        fld dword ptr [eax + 0ch]
        fstp dword ptr [esi + 0ch]
        mov edx, dword ptr [eax + 10h]
        mov dword ptr [esi + 10h], edx
        fld dword ptr [eax + 14h]
        mov edx, dword ptr [edi + 34h]
        fstp dword ptr [esi + 14h]
        mov dword ptr [esi + 1ch], ecx
        mov dword ptr [esi + 18h], edx
        mov dword ptr [esi + 20h], 0
        movss xmm0, dword ptr [eax + 14h]
        lea eax, [edi + 1ch]
        mov dword ptr [edi + 10h], eax
        lea ecx, [edi + 24h]
        mov dword ptr [edi + 0ch], ecx
        movss dword ptr [edi + 1ch], xmm0
    }
    insert_native_gui_camera_store_00aa4960(at(manager, 8), frame.insert_00);
    acquired.inserted = true;
    const char* const name = camera_name_data(pointer(store, 0x18), bindings.actual_empty_00f8bc60);
    (void)name; // 4254B0 is RET; its native name-data load still occurs above.
    return store;
}
} // namespace bsp
