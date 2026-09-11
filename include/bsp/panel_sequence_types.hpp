#pragma once

#include "bsp/native_string.hpp"

#include <map>
#include <vector>

namespace bsp {
// Descriptive projections, not native command or container layouts. The
// native kind3 float+8 overlaps the kind0/1/4 NativeString+8. Only the payload
// selected by the CURRENT virtual kind result is meaningful. The caller owns
// the command allocation; destroy_vslot_00(1) must actually delete it.
struct PanelSequenceCommand {
    virtual ~PanelSequenceCommand() = default;
    virtual void destroy_vslot_00(std::uint32_t deleting) = 0;
    virtual std::int32_t kind_vslot_04() = 0;
    std::uint32_t word_04{};
    NativeString string_08;
    NativeString string_10;
    float time_08{};
};
struct PanelSequenceSlot {
    std::uint8_t active_00{};
    NativeString text_04;
    NativeString palette_0c;
};
// These initial values are the actual default entry at00451974..004519B5.
// Native entry40h: command vector+14, cursor+24, slot vector+28.
struct PanelSequenceEntry {
    std::uint32_t native_vtable_00{0x00ce4b40};
    float priority_04{};
    float tie_08{};
    float elapsed_0c{};
    std::uint8_t flag_10{};
    std::vector<PanelSequenceCommand*> commands_18;
    std::uint32_t cursor_24{};
    std::vector<PanelSequenceSlot> slots_2c;
    std::uint8_t erase_38{};
    std::uint32_t selected_slot_3c{};
};

// Complete00443D00/00449AF0 wrappers around the actual host CRT _stricmp.
// Length-zero gates precede CRT comparison; embedded NUL and locale semantics
// are CRT semantics, not a Unicode or length-bounded comparison.
bool native_string_less_case_insensitive_00443d00(const NativeString&, const NativeString&);
bool native_string_not_equal_case_insensitive_00449af0(const NativeString&, const NativeString&);
struct PanelSequenceNameLess {
    bool operator()(const NativeString& a, const NativeString& b) const {
        return native_string_less_case_insensitive_00443d00(a, b);
    }
};
using PanelSequenceQueue = std::map<NativeString, PanelSequenceEntry, PanelSequenceNameLess>;


} // namespace bsp
