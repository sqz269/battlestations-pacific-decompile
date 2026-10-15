#include "bsp/native_input_pooled_key_subtrees.hpp"
#include "bsp/native_string.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == sizeof(Word));
Word address(const void* value) noexcept { return reinterpret_cast<Word>(value); }
void* pointer(Word value) noexcept { return reinterpret_cast<void*>(value); }
Word read(Word base, Word byte_offset) noexcept {
    Word value;
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov eax, dword ptr [eax + edx] }
    __asm { mov value, eax }
    return value;
}
unsigned char nil_byte(Word node) noexcept {
    unsigned char value;
    __asm { mov eax, node }
    __asm { mov al, byte ptr [eax + 25h] }
    __asm { mov value, al }
    return value;
}
void write(Word base, Word byte_offset, Word value) noexcept {
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov ecx, value }
    __asm { mov dword ptr [eax + edx], ecx }
}
} // namespace

void __fastcall destroy_native_input_word_vector_subtree_00558310(void* tree,
    Word, void* node, NativeStringRawPoolContext& strings) {
    Word current = address(node);
    bool is_nil = nil_byte(current) != 0u;
    while (!is_nil) {
        const Word right = read(current, 8u);
        destroy_native_input_word_vector_subtree_00558310(tree, 0u,
            pointer(right), strings);
        const Word mapped_backing = read(current, 0x18u);
        const bool has_backing = mapped_backing != 0u;
        const Word captured_left = read(current, 0u);
        if (has_backing) singleton_lifetime_free(pointer(mapped_backing));
        write(current, 0x18u, 0u);
        write(current, 0x1cu, 0u);
        write(current, 0x20u, 0u);
        // Full raw41DD20 reads CURRENT data once, then conditional length+1
        // before the getter. No key reread/clear or noexcept bridge is added.
        destroy_native_string_header_0041dd20(pointer(current + 0x0cu), strings);
        singleton_lifetime_free(pointer(current));
        is_nil = nil_byte(captured_left) != 0u;
        current = captured_left;
    }
}

void __fastcall destroy_native_input_descriptor_vector_subtree_00558e60(
    void* tree, Word unused_edx, void* node, NativeStringRawPoolContext& strings) {
    destroy_native_input_word_vector_subtree_00558310(tree, unused_edx, node, strings);
}
} // namespace bsp
