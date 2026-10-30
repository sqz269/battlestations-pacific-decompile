#include "bsp/native_font_glyph_selection_storage.hpp"
#include "bsp/native_font_storage.hpp"
#include "bsp/native_invalid_parameter_noinfo_call.hpp"
#include <cstddef>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4);
static_assert(sizeof(NativeFontGlyphSelectionStorageScratch) == 8);
static_assert(alignof(NativeFontGlyphSelectionStorageScratch) == 4);

Word bits(const volatile void* p) noexcept {
    return static_cast<Word>(reinterpret_cast<std::uintptr_t>(p));
}
void* pointer(Word value) noexcept {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(value));
}
void* at(const volatile void* p, Word offset) noexcept {
    return pointer(bits(p) + offset); // actual wrapping DWORD address arithmetic
}
Word load(const volatile void* p, Word offset = 0) noexcept {
    return *static_cast<const volatile Word*>(at(p, offset));
}

// Complete normal AD3E70..AD3E93, privately composed only for this consumer.
// No helper/vendor/Original body credit. SDK handler is not assumed noreturn;
// do not cache owner/node/head across its calls or add guards after it returns.
void* current_iterator_pair_00ad3e70(void* actual_iterator8) {
    if (load(actual_iterator8) == 0) invoke_native_invalid_parameter_00bf6713();
    void* const current_owner = pointer(load(actual_iterator8)); // AD3E7D, FRESH
    const Word compared_node = load(actual_iterator8, 4);         // AD3E7F
    if (compared_node == load(current_owner, 4))
        invoke_native_invalid_parameter_00bf6713();
    const Word current_node = load(actual_iterator8, 4);          // AD3E8C, FRESH
    return pointer(current_node + 0x0cu);                        // actual key/value pair
}
} // namespace

__declspec(noinline) void* select_native_font_glyph_storage_00ad4480(
    void* captured_actual_font, const volatile std::uint16_t& actual_code_unit_home,
    NativeFontGlyphSelectionStorageScratch& actual_scratch) {
    const std::uint16_t initial_key = actual_code_unit_home; // AD4480, FIRST
    if (initial_key == 0x20u || initial_key == 0x0au)
        return at(captured_actual_font, 0x4c);
    if (initial_key == 0x0du) return at(captured_actual_font, 0x8c);

    void* const iterator8 = &actual_scratch.iterator_words[0];
    // Do not pass initial_key: AD4370 performs its own conditional/current
    // WORD reads from the SAME actual argument home, including late equality.
    (void)find_native_font_glyph_iterator_00ad4370(captured_actual_font,
        iterator8, actual_code_unit_home);
    const Word owner = load(iterator8);                        // AD44BC
    const Word captured_head = load(captured_actual_font, 4);   // AD44C2, BEFORE guard
    if (owner == 0 || owner != bits(captured_actual_font))
        invoke_native_invalid_parameter_00bf6713();
    // The handler may have changed current iterator storage. Native still
    // compares it with the captured pre-handler head, not a reloaded Font head.
    if (load(iterator8, 4) == captured_head)
        return at(captured_actual_font, 0x6c);

    void* const pair = current_iterator_pair_00ad3e70(iterator8);
    return pointer(load(pair, 4)); // actual CURRENT payload pointer; NULL stays NULL
}
} // namespace bsp
