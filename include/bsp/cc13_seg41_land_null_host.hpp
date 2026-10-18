#pragma once
// A LandVehicleHost whose every hook records that it was reached and answers
// zero (null, false, 0). Generated from include/bsp/cc13_seg41_land.hpp by the
// same rule as include/bsp/cc13_big1_null_host.hpp: an adapter derives from it
// and overrides only the hooks its body reaches, so a hook a body was not
// expected to reach shows up in unreached() instead of running. Not a
// reconstruction: no hook here models the image.

#include "bsp/cc13_seg41_land.hpp"

#include <cstdint>

namespace bsp::cc13_seg41 {

class LandVehicleNullHost : public LandVehicleHost {
public:
    // Called with the hook's name and the address its name carries (0 when
    // the name has none) whenever a hook that was not overridden runs.
    virtual void unreached(const char* hook, std::uint32_t address) = 0;

    long interlocked_decrement(volatile long*) override {
        unreached("interlocked_decrement", 0u); return {};
    }
    void release_slot0(RefCounted*) override {
        unreached("release_slot0", 0u);
    }
    long interlocked_increment(volatile long*) override {
        unreached("interlocked_increment", 0u); return {};
    }
    void attach_lua_self_0077e830(std::uint8_t*) override {
        unreached("attach_lua_self_0077e830", 0x0077e830u);
    }
    void instantiate_class_devices_0095a880(
        std::uint8_t*, std::uint32_t, std::uint32_t, std::uint32_t) override {
        unreached("instantiate_class_devices_0095a880", 0x0095a880u);
    }
    void convoy_member_died_007422a0(std::uint8_t*, std::uint8_t*) override {
        unreached("convoy_member_died_007422a0", 0x007422a0u);
    }
    void unit_on_death_00951fb0(std::uint8_t*) override {
        unreached("unit_on_death_00951fb0", 0x00951fb0u);
    }
    void base_destruct_007668e0(GroundVehicleCreateMessage&) override {
        unreached("base_destruct_007668e0", 0x007668e0u);
    }
    void send_to_peer_00779fc0(std::uint8_t*, std::uint32_t, void*) override {
        unreached("send_to_peer_00779fc0", 0x00779fc0u);
    }
    void release_peer_00779f80(std::uint8_t*, const std::uint32_t*) override {
        unreached("release_peer_00779f80", 0x00779f80u);
    }
    std::uint8_t party_00927850(std::uint8_t*, std::uint32_t) override {
        unreached("party_00927850", 0x00927850u); return {};
    }
    void build_sync_0077b280(
        std::uint8_t*, const std::uint32_t*, std::uint8_t, float, std::uint32_t, float,
        std::uint32_t, float, std::uint32_t, float, std::uint32_t, float, std::uint32_t,
        float, std::uint32_t) override {
        unreached("build_sync_0077b280", 0x0077b280u);
    }
    void matrix_set_004134f0(std::uint8_t*, const float*) override {
        unreached("matrix_set_004134f0", 0x004134f0u);
    }
    void motion_resync_008746a0(std::uint8_t*, std::uint32_t) override {
        unreached("motion_resync_008746a0", 0x008746a0u);
    }
    void motion_step_slot04(std::uint8_t*, float) override {
        unreached("motion_step_slot04", 0u);
    }
    void points_reserve_0074d190(std::uint8_t*, std::int32_t) override {
        unreached("points_reserve_0074d190", 0x0074d190u);
    }
    void unit_serialize_0095bb10(std::uint8_t*, ArchiveWriter&) override {
        unreached("unit_serialize_0095bb10", 0x0095bb10u);
    }
    std::uint8_t* vehicle_class_00964790(std::uint16_t, bool) override {
        unreached("vehicle_class_00964790", 0x00964790u); return {};
    }
    std::uint8_t* class_create_unit_28(std::uint8_t*, std::uint16_t) override {
        unreached("class_create_unit_28", 0u); return {};
    }
    void native_string_resize_0041dd40(std::uint8_t*, std::uint32_t, std::uint32_t) override {
        unreached("native_string_resize_0041dd40", 0x0041dd40u);
    }
    void memcpy_00bf7680(void*, const void*, std::size_t) override {
        unreached("memcpy_00bf7680", 0x00bf7680u);
    }
    void class_bind_base_0095f500(std::uint8_t*) override {
        unreached("class_bind_base_0095f500", 0x0095f500u);
    }
    void std_string_assign_00408720(std::uint8_t*, const char*, std::uint32_t) override {
        unreached("std_string_assign_00408720", 0x00408720u);
    }
    std::uint8_t* model_find_node_00718870(std::uint8_t*, const std::uint8_t*, std::uint32_t) override {
        unreached("model_find_node_00718870", 0x00718870u); return {};
    }
    void free_00bf65ac(void*) override {
        unreached("free_00bf65ac", 0x00bf65acu);
    }
    void free_00bf6989(void*) override {
        unreached("free_00bf6989", 0x00bf6989u);
    }
    void invalid_iterator_00bf6713() override {
        unreached("invalid_iterator_00bf6713", 0x00bf6713u);
    }
    void refresh_pose_00414db0(std::uint8_t*) override {
        unreached("refresh_pose_00414db0", 0x00414db0u);
    }
    RefCounted** effect_create_at_008685e0(
        RefCounted**, std::uint32_t, RefCounted*, const float*, std::uint32_t,
        std::uint32_t) override {
        unreached("effect_create_at_008685e0", 0x008685e0u); return {};
    }
    void sound_play_0047bb70(std::uint32_t, std::uint32_t, std::uint8_t*) override {
        unreached("sound_play_0047bb70", 0x0047bb70u);
    }
    void unit_destroyed_00959450(std::uint8_t*) override {
        unreached("unit_destroyed_00959450", 0x00959450u);
    }
    void effect_stop_00867b10(std::uint8_t*) override {
        unreached("effect_stop_00867b10", 0x00867b10u);
    }
    void effect_vector_resize_004c9550(std::uint8_t*, std::int32_t) override {
        unreached("effect_vector_resize_004c9550", 0x004c9550u);
    }
    void kill_00926d90(std::uint8_t*, std::uint32_t) override {
        unreached("kill_00926d90", 0x00926d90u);
    }
    void unit_base_construct_0095cc90(std::uint8_t*, std::uint32_t, std::uint32_t) override {
        unreached("unit_base_construct_0095cc90", 0x0095cc90u);
    }
    void unit_base_destruct_00959940(std::uint8_t*) override {
        unreached("unit_base_destruct_00959940", 0x00959940u);
    }
    void ai_query_008ddf00(std::uint32_t, std::uint8_t*) override {
        unreached("ai_query_008ddf00", 0x008ddf00u);
    }
    void unit_detach_base_006d3620(std::uint8_t*) override {
        unreached("unit_detach_base_006d3620", 0x006d3620u);
    }
    void list_erase_004837d0(std::uint8_t*, std::uint8_t*) override {
        unreached("list_erase_004837d0", 0x004837d0u);
    }
    void* operator_new_00bf55be(std::size_t) override {
        unreached("operator_new_00bf55be", 0x00bf55beu); return {};
    }
    void memset_00bf79f0(void*, int, std::size_t) override {
        unreached("memset_00bf79f0", 0x00bf79f0u);
    }
    void set_vehicle_class_009553d0(std::uint8_t*, std::uint32_t) override {
        unreached("set_vehicle_class_009553d0", 0x009553d0u);
    }
    void unit_load_common_0095e5b0(std::uint8_t*) override {
        unreached("unit_load_common_0095e5b0", 0x0095e5b0u);
    }
    void saved_lua_data_009238a0(std::uint8_t*, std::uint8_t (&)[0x14]) override {
        unreached("saved_lua_data_009238a0", 0x009238a0u);
    }
    void lua_reader_construct_004425c0(std::uint8_t*, const std::uint8_t (&)[0x14]) override {
        unreached("lua_reader_construct_004425c0", 0x004425c0u);
    }
    void lua_reader_enter_00bd8e20(std::uint8_t*, VariantPair) override {
        unreached("lua_reader_enter_00bd8e20", 0x00bd8e20u);
    }
    void lua_reader_read_10(std::uint8_t*, VariantPair, VariantPair) override {
        unreached("lua_reader_read_10", 0u);
    }
    void lua_reader_read_00bd6830(std::uint8_t*, VariantPair, VariantPair) override {
        unreached("lua_reader_read_00bd6830", 0x00bd6830u);
    }
    void lua_reader_leave_00bd7a20(std::uint8_t*) override {
        unreached("lua_reader_leave_00bd7a20", 0x00bd7a20u);
    }
    void lua_reader_destroy_00441a20(std::uint8_t*) override {
        unreached("lua_reader_destroy_00441a20", 0x00441a20u);
    }
    void unit_vslot_148(std::uint8_t*, std::uint32_t, std::uint32_t) override {
        unreached("unit_vslot_148", 0u);
    }
};

} // namespace bsp::cc13_seg41
