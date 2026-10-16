#include "bsp/native_input_preset_key_subtree.hpp"
#include "bsp/native_string_pool_storage.hpp"
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
    __asm { mov al, byte ptr [eax + 29h] }
    __asm { mov value, al }
    return value;
}
} // namespace

void __fastcall destroy_native_input_preset_key_subtree_006a0770(
    void* tree, Word, void* node, NativeStringRawPoolContext& strings) {
    Word current = address(node);
    bool is_nil = nil_byte(current) != 0u;
    while (!is_nil) {
        const Word right = read(current, 8u);
        destroy_native_input_preset_key_subtree_006a0770(
            tree, 0u, pointer(right), strings);
        const Word key_data = read(current, 0x10u);
        const bool has_key_data = key_data != 0u;
        const Word captured_left = read(current, 0u);
        if (has_key_data) {
            const Word size = read(current, 0x0cu) + 1u;
            auto* const pool = native_string_pool_get_or_create_00419cc0(
                strings.actual_published_01090aa8,
                strings.actual_manager_publication_01090aa0);
            return_native_string_pool_00bd1510(pool, pointer(key_data), size,
                strings.actual_small_returns_disabled_01090aa4);
        }
        singleton_lifetime_free(pointer(current));
        is_nil = nil_byte(captured_left) != 0u;
        current = captured_left;
    }
}
} // namespace bsp
