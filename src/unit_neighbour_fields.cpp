#include "bsp/unit_neighbour_fields.hpp"

#include <cstring>

namespace bsp {
namespace {

std::uint32_t float_word(const float& value) noexcept {
    std::uint32_t word;
    std::memcpy(&word, &value, sizeof(word));
    return word;
}

void store_float_word(float& destination, std::uint32_t word) noexcept {
    std::memcpy(&destination, &word, sizeof(word));
}

} // namespace

void bind_unit_dummy_object_00953a80(UnitDummyBindingHost& host, void* unit,
                                   std::int32_t dummy_object_id) {
    auto destination = host.unit_fields(unit);
    destination.dummy_object_id_06b8 = dummy_object_id; // 00953A87, before lookup
    for (void* node = host.world_list45_head(); node != nullptr;
         node = host.list_next_0004(node)) {
        void* const dummy = host.list_value_0008(node);
        auto source = host.dummy_fields(dummy);
        if (source.dummy_object_id_0484 != dummy_object_id) {
            continue;
        }

        void* const frontend = host.frontend_00e198c4();
        if (host.frontend_mode_0020(frontend) == 0x26 &&
            host.selected_dummy_0398(frontend) == dummy) {
            void* const selected = host.unit_virtual_0140(unit); // 00953AF7
            void* const current_frontend = host.frontend_00e198c4();
            void* const hud = host.hud_root_0040(current_frontend);
            host.set_spectated_unit_00647300(hud, selected); // 00953B03
        }

        constexpr std::uint32_t one = 0x3f800000; // 00D7A24C
        std::uint32_t first = one;
        if (host.visibility_override_00f87152() == 0 &&
            source.visibility_override_02f0 == 0) {
            first = float_word(source.visibility_02f4);
        }
        store_float_word(destination.visibility_02f4, first); // 00953B2F
        std::uint32_t second = one;
        if (host.visibility_override_00f87152() == 0 &&
            source.visibility_override_02f0 == 0) {
            second = float_word(source.visibility_02f8);
        }
        store_float_word(destination.visibility_02f8, second); // 00953B55
        host.set_dummy_visible_006e0b40(dummy, false); // 00953B5D
        if (host.session_mode_1fe4() != 2) {
            host.kill_entity_00926d90(dummy, 2); // 00953B75
        }
        return;
    }
}

} // namespace bsp
