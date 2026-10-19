#include "bsp/oct18b_nA1_early.hpp"

#include <cstring>

namespace bsp::oct18b_nA1 {

void clamp_int_in_place_00415660(std::int32_t& value, const std::int32_t& minimum,
                                const std::int32_t& maximum) noexcept {
    const std::int32_t original = value;
    const std::int32_t low = minimum;
    if (original < low) value = low;
    else {
        const std::int32_t high = maximum;
        value = original > high ? high : original;
    }
}

void key_table_find_bracket_0043a8b0(KeyTableView& table, float key, std::uint32_t* lower,
                                    std::uint32_t* upper, KeyBracketHost& host) {
    auto count = [&]() -> std::uint32_t {
        return table.begin ? static_cast<std::uint32_t>(table.end - table.begin) : 0;
    };
    auto checked = [&](std::uint32_t index) -> float {
        if (!table.begin || index >= count()) host.invalid_parameter_00bf6713();
        return table.begin[index].key;
    };
    if (!table.begin || count() == 0) host.invalid_parameter_00bf6713();
    // The first FCOMIP/JC also takes the search branch when unordered.
    if (table.begin[0].key >= key) {
        *upper = 0;
        *lower = 0;
        return;
    }
    *upper = count() - 1u;
    // FCOMIP key,last / JC: unordered follows the strict-less branch.
    if (key >= checked(*upper)) {
        *lower = *upper;
        return;
    }
    *lower = 0;
    while (static_cast<std::int32_t>(*lower) < static_cast<std::int32_t>(*upper - 1u)) {
        const auto middle = static_cast<std::uint32_t>(static_cast<std::int32_t>(*lower + *upper) >> 1);
        if (key >= checked(middle)) *lower = middle;
        else *upper = middle;
    }
}

namespace {
std::uint32_t word_at(const void* object, std::uint32_t offset) noexcept {
    std::uint32_t value;
    std::memcpy(&value, static_cast<const std::uint8_t*>(object) + offset, sizeof value);
    return value;
}
}
std::uint32_t opaque_word_at_08_0047b960(const void* object) noexcept { return word_at(object, 8); }
std::uint32_t opaque_word_at_04_004b6580(const void* object) noexcept { return word_at(object, 4); }
void opaque_empty_callback_00482800(void*) noexcept {}
void opaque_empty_callback_004c8620(void*, void*) noexcept {}

}
