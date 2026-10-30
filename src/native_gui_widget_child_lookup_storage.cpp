#include "bsp/native_gui_widget_child_lookup_storage.hpp"
#include "bsp/native_invalid_parameter_noinfo_call.hpp"
#include <cstring>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4);
Word bits(const void* p) noexcept {
    return static_cast<Word>(reinterpret_cast<std::uintptr_t>(p));
}
void* pointer(Word value) noexcept {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(value));
}
Word load(Word address, Word field_offset = 0) {
    Word value;
    __asm { mov eax, address }
    __asm { mov edx, field_offset }
    __asm { mov eax, dword ptr [eax + edx] }
    __asm { mov value, eax }
    return value;
}
// Exact private B6D800 LEA/RET primitive. No typed NativeNodeStorage cast,
// dereference, replacement header, helper/Original/vendor body credit.
Word node_name_address_00b6d800(Word captured_actual_node) noexcept {
    return captured_actual_node + 0x54u;
}
} // namespace

__declspec(noinline) void* find_native_gui_widget_child_storage_00aa7e00(
    void* captured_actual_parent, volatile Word (&actual_homes)[2]) {
    const Word parent = bits(captured_actual_parent);
    const Word initial_head = load(parent, 0x68); // AA7E00, BEFORE query home.
    const Word query = actual_homes[0];           // AA7E05: ONE pointer capture.
    const Word list = parent + 0x64u;             // Native EBX.
    Word sibling = load(initial_head);           // AA7E0D: first next pointer.
    for (;;) {
        // CMP EBX,EBX makes AA7E17 unreachable, even if list bits wrap to0.
        // Its intervening current-head load is still required on every visit.
        const Word current_end = load(list, 4);   // AA7E12.
        if (sibling == current_end) return nullptr;
        if (sibling == load(list, 4))             // AA7E24, FRESH head.
            invoke_native_invalid_parameter_00bf6713(); // AA7E29; may return.

        const Word tested_child = load(sibling, 8);
        if (load(tested_child, 0x4c) != 0) {
            if (sibling == load(list, 4))         // AA7E37, FRESH head.
                invoke_native_invalid_parameter_00bf6713(); // AA7E3C.
            const Word current_child = load(sibling, 8); // AFTER guard callback.
            const Word current_node = load(current_child, 0x4c);
            const Word name = node_name_address_00b6d800(current_node); // AA7E47.
            const Word query_length = load(query);       // AA7E4C, FIRST.
            const Word child_length = load(name);        // AA7E4F, SECOND.
            bool equal = false;
            if (child_length == query_length) {
                if (child_length == 0) equal = true; // Native zero arm, no data read.
                else {
                    const auto* const query_data = static_cast<const char*>(pointer(load(query, 4)));
                    const auto* const child_data = static_cast<const char*>(pointer(load(name, 4)));
                    equal = _stricmp(child_data, query_data) == 0; // AA7E72.
                }
            }
            if (equal) {
                if (sibling == load(list, 4))     // AA7E94, FRESH head.
                    invoke_native_invalid_parameter_00bf6713(); // AA7E99.
                return pointer(load(sibling, 8)); // AA7E9E, CURRENT payload.
            }
        }
        if (sibling == load(list, 4))             // AA7E83, FRESH head.
            invoke_native_invalid_parameter_00bf6713(); // AA7E88.
        sibling = load(sibling);                 // AA7E8D, CURRENT next.
    }
}
} // namespace bsp
