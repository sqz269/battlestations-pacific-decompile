// Segment 41 tail rows (packet cc15-rC1). Evidence and ABI:
// include/bsp/cc15_seg41_tail.hpp and docs/CC15_SEG41_TAIL.md.
#include "bsp/cc15_seg41_tail.hpp"

#include <cstring>

namespace bsp::cc15_seg41_tail {
namespace {

template <typename T>
void store(std::uint8_t* object, std::uint32_t offset, T value) {
    std::memcpy(object + offset, &value, sizeof value);
}

} // namespace

// 00748A40-00748AF4. The EH frame (handler 00C873C6) only matters if 00926BE0
// or 00748990 throws; that unwind path is not modelled.
std::uint8_t* stationary_prop_construct_00748a40(std::uint8_t* prop,
                                                StationaryPropConstructHost& host) {
    host.scene_node_construct_00925ce0(prop);                   // 00748A5F
    store<std::uint32_t>(prop, 0x00, kStationaryPropVtable00);  // 00748A64
    store<std::uint32_t>(prop, 0x10, kStationaryPropVtable10);  // 00748A6A
    store<std::uint32_t>(prop, 0x24, kStationaryPropVtable24);  // 00748A71
    store<std::uint32_t>(prop, 0x184, 0u);                      // 00748A80
    store<std::uint32_t>(prop, 0xC4, kStationaryPropKindC4);    // 00748A91
    host.sentity_push_pending_00926be0(prop);                   // 00748A9B

    // 00748AA0-00748AB8: the prop's index is the vector's size before the
    // push, zero while the begin word is null.
    const std::uintptr_t begin = host.prop_vector_begin_00f87560();
    std::uint32_t index = 0;
    if (begin != 0) {
        const std::uintptr_t end = host.prop_vector_end_00f87564();
        index = static_cast<std::uint32_t>(static_cast<std::int32_t>(end - begin) >> 2);  // SAR
    }
    store<std::uint32_t>(prop, 0x174, index);

    std::uint8_t* const slot = prop;                            // [ESP+4]
    host.prop_vector_push_back_00748990(&slot);                 // 00748ACC
    store<std::uint8_t>(prop, 0x1A4, 0u);                       // 00748AD8
    store<float>(prop, 0x170, 0.0f);                            // 00748ADF, XORPS/MOVSS
    return prop;
}

void net_entity_create_write_thunk_0074cca0(void* message, void* cursor, TailThunkHost& host) {
    host.net_entity_create_write_00765000(message, cursor);
}

void game_entity_place_in_world_thunk_0074e180(void* entity, void* hierarchy_parent,
                                               void* world_node, const void* matrix,
                                               TailThunkHost& host) {
    host.game_entity_place_in_world_00928860(entity, hierarchy_parent, world_node, matrix);
}

bool ship_class_derive_turn_fields_thunk_00758140(void* descriptor, TailThunkHost& host) {
    return host.ship_class_derive_turn_fields_00828f20(descriptor);
}

void ship_on_health_changed_thunk_00758390(void* ship, TailThunkHost& host) {
    host.ship_on_health_changed_00827a90(ship);
}

} // namespace bsp::cc15_seg41_tail
