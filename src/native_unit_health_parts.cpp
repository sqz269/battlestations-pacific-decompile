#include "bsp/native_unit_health_parts.hpp"
#include <cstring>

namespace bsp {
namespace {
std::uint32_t address(const void* p) noexcept
{
    return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(p));
}
std::int32_t signed_word(std::uint32_t word) noexcept
{
    std::int32_t result;
    std::memcpy(&result, &word, sizeof(result));
    return result;
}
std::uint32_t part_count(NativeUnitClassPartsView c) noexcept
{
    // IMUL 2AAAAAAB / SAR3 / correction implements signed /48, then the
    // caller compares the result as unsigned. Preserve DWORD subtraction.
    const auto begin = c.parts_begin_1c;
    if (!begin) return 0;
    return static_cast<std::uint32_t>(signed_word(address(c.parts_end_20) -
        address(begin)) / static_cast<std::int32_t>(kVehicleClassPartDescStride));
}
std::uint32_t pointer_count(NativeUnitPartPointerVectorView v) noexcept
{
    const auto begin = v.begin_04;
    if (!begin) return 0;
    return static_cast<std::uint32_t>(signed_word(address(v.end_08) - address(begin)) >> 2);
}
}

void initialize_native_unit_health_parts_0087bcc0(NativeUnitHealthPartsView u,
    NativeUnitHealthPartsGlobals g, NativeUnitHealthPartsBindings& b)
{
    void* const unit = u.unit.canonical_unit;
    b.call_0077f0e0(unit);
    auto initial_class = b.class_parts(u.descriptor_354);
    const float armour = initial_class.armour_4c;
    const float one = g.one_00d7a24c;
    u.armour_368 = armour;
    const float hp = initial_class.hp_48;
    u.maximum_health_36c = hp;
    u.marker_164 = one;
    u.health_370 = hp;
    u.condition_378 = 1;
    b.call_0087b460(u.parts_344.identity, part_count(initial_class), 0);

    std::uint32_t index = 0;
    std::uint32_t offset = 0;
    while (u.parts_344.begin_04 && index < pointer_count(u.parts_344)) {
        auto current_class = b.class_parts(u.descriptor_354);
        if (!current_class.parts_begin_1c || index >= part_count(current_class))
            b.call_00bf6713();
        // EDI remains the captured class vector across a returning trap.
        void* const part = reinterpret_cast<void*>(static_cast<std::uintptr_t>(
            address(current_class.parts_begin_1c) + offset));
        if (!u.parts_344.begin_04 || index >= pointer_count(u.parts_344))
            b.call_00bf6713();
        void* const destination = reinterpret_cast<void*>(static_cast<std::uintptr_t>(
            address(u.parts_344.begin_04) + index * 4u)); // reload after trap
        std::memcpy(destination, &part, 4); // native DWORD address wrap
        ++index;
        offset += static_cast<std::uint32_t>(kVehicleClassPartDescStride);
    }

    void* parent = u.parent_3c;
    while (parent) {
        auto entry = b.primary_table(parent)[0xb0 / 4];
        if (b.call_parent_b0(entry, parent) != 0) {
            entry = b.primary_table(parent)[0xb0 / 4];
            (void)b.call_parent_b0(entry, parent); // deliberately called twice
            break;
        }
        parent = b.parent_3c(parent);
    }

    void* const part_set = b.class_parts(u.descriptor_354).part_set_50;
    if (!part_set) {
        u.model_360 = nullptr;
        return;
    }
    const auto kind_entry = b.primary_table(unit)[0x5c / 4];
    const bool kind_1b = b.call_unit_5c(kind_entry, unit, 0x1b) != 0;
    void* const allocation = b.call_00bf681b(kUnitPartInstanceSize);
    void* model = nullptr;
    try {
        if (allocation) {
            const auto unit_table = b.primary_table(unit);
            float detail;
            if (kind_1b) {
                const float* input = &g.detail_00ced9e0;
                __asm {
                    mov eax, input
                    fld dword ptr [eax]
                }
            } else {
                __asm fld1
            }
            // Pure table access preserves the x87 stack/environment. Native
            // keeps the loaded detail on ST0 across these two table reads.
            const auto part_table = b.primary_table(part_set);
            const auto detail_entry = unit_table[0x190 / 4];
            __asm fstp dword ptr [detail]
            const auto selector = b.call_unit_190(detail_entry, unit);
            // Capture table before +190; read its +8 ENTRY after +190 returns.
            void* const selected = b.call_part_set_08(part_table[8 / 4], part_set,
                selector, detail);
            model = b.call_007135c0(allocation, unit, selected);
        }
    } catch (...) {
        // C967D0/C967DB both free the SAVED allocation [EBP-10]. Original
        // native FH3/SEH dispatch is not emulated by this source C++ catch.
        b.call_00bf65ac(allocation);
        throw;
    }
    u.model_360 = model;

    std::int32_t numbering;
    void* const holder = u.property_holder_c0;
    if (holder && b.property_holder(holder).kind_04 == 2) {
        void* const saved_identity = b.property_holder(holder).bag_08;
        auto saved = b.saved_parts(saved_identity);
        b.call_00876ec0(unit, saved.state_58);
        numbering = saved.numbering_5c; // same saved object, live after callback
    } else {
        b.call_00876ec0(unit, 0);
        numbering = 1;
        void* const current_holder = u.property_holder_c0;
        if (current_holder) {
            void* const bag = b.property_holder(current_holder).bag_08;
            if (bag) {
                void* const property = b.call_008f2260(bag, "Numbering");
                if (property) numbering = b.property_integer_0c(property);
            }
        }
    }
    if (u.numbering_35c == numbering) return;
    const bool had_model = u.model_360 != nullptr; // CMP before the +35C store
    u.numbering_35c = numbering;
    if (!had_model) return;
    auto entry = b.primary_table(unit)[0x5c / 4];
    if (!b.call_unit_5c(entry, unit, 6)) {
        entry = b.primary_table(unit)[0x5c / 4];
        if (!b.call_unit_5c(entry, unit, 0x1b)) return;
    }
    b.call_00711be0(u.model_360, numbering); // live model after virtual calls
}
} // namespace bsp
