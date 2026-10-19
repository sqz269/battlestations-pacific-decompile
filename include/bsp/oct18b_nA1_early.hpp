#pragma once

#include <cstdint>

// Newly defined leaves with no established class family. These explicit C++
// interfaces are build-only models, not binary-compatible entry points.
namespace bsp::oct18b_nA1 {

// 00415660: ECX value ref, EDX minimum ref, stack maximum ref, RET 4.
// Signed integer clamp; aliases are read in the same order as the listing.
void clamp_int_in_place_00415660(std::int32_t& value, const std::int32_t& minimum,
                                const std::int32_t& maximum) noexcept;

struct KeyValue8 { float key; std::uint32_t value; };
struct KeyTableView { void* proxy; KeyValue8* begin; KeyValue8* end; KeyValue8* capacity; };
static_assert(sizeof(KeyTableView) == 16);
struct KeyBracketHost {
    virtual ~KeyBracketHost() = default;
    virtual void invalid_parameter_00bf6713() = 0;
};
// 0043A8B0: ECX table, stack float/lower*/upper*, RET 0Ch. Keys are at
// 8-byte strides. NaN follows the listing's CF branches; no sortedness repair.
void key_table_find_bracket_0043a8b0(KeyTableView& table, float key, std::uint32_t* lower,
                                    std::uint32_t* upper, KeyBracketHost& host);

// ECX opaque object, RET, EAX dword. Only the field access is established;
// tables 00D2A474/00D2AC74 and 00D2E060 do not establish a game class.
std::uint32_t opaque_word_at_08_0047b960(const void* object) noexcept;
std::uint32_t opaque_word_at_04_004b6580(const void* object) noexcept;

// Exact empty bodies: RET 4 and RET 8. Their data-slot owners remain unknown.
void opaque_empty_callback_00482800(void* argument) noexcept;
void opaque_empty_callback_004c8620(void* first, void* second) noexcept;

}
