#pragma once
// A base for host adapters over bsp::UnitDamageHost (include/bsp/unit_damage.hpp).
//
// UnitDamageHost carries 80 engine services as pure virtuals and no game host
// implemented it, so the six Lua handlers and the damage routines of
// src/unit_damage.cpp were unreachable from a host root. This class overrides
// every one of them with a loud failure; an adapter derives from it and
// overrides only the services its own path reaches, each mapped onto the host's
// real objects. A service the adapter does not override throws
// std::logic_error naming the service, so a path the adapter did not read fails
// at once instead of running on an invented value.
//
// Written mechanically from the declarations in unit_damage.hpp (goal 5 chain
// 649, local generator over the struct's `virtual ... = 0;` lines), one
// override per pure virtual, same signature, in the header's order, as
// bsp/fe5_host_unreached.hpp does for Fe5Host. A C++ adapter base, not an image
// type: no address, no image layout. Keep it in step with UnitDamageHost.

#include "bsp/unit_damage.hpp"

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>

namespace bsp {

[[noreturn]] inline void unit_damage_unreached(const char* service) {
    throw std::logic_error(std::string("UnitDamageHost service not provided by this adapter: ") + service);
}

// One macro body, so nothing reading the sources for implementations of a
// service (tools/wire_readiness.py among them) takes one for an implementation.
#define BSP_UNIT_DAMAGE_UNREACHED { ::bsp::unit_damage_unreached(__func__); }
#pragma warning(push)
#pragma warning(disable : 4100)  // the parameter names document the services
class UnitDamageHostUnreached : public UnitDamageHost {
public:
    std::uint32_t entity_from_lua_table(int argument_index) override BSP_UNIT_DAMAGE_UNREACHED
    int lua_argument_count() override BSP_UNIT_DAMAGE_UNREACHED
    float lua_number_argument(int index) override BSP_UNIT_DAMAGE_UNREACHED
    bool lua_boolean_argument(int index) override BSP_UNIT_DAMAGE_UNREACHED
    bool lua_argument_is_boolean(int index) override BSP_UNIT_DAMAGE_UNREACHED
    void lua_push_boolean(bool value) override BSP_UNIT_DAMAGE_UNREACHED
    UnitHealth read_health(std::uint32_t entity) override BSP_UNIT_DAMAGE_UNREACHED
    bool entity_released(std::uint32_t entity) override BSP_UNIT_DAMAGE_UNREACHED
    bool entity_release_requested(std::uint32_t entity) override BSP_UNIT_DAMAGE_UNREACHED
    bool entity_killed(std::uint32_t entity) override BSP_UNIT_DAMAGE_UNREACHED
    bool entity_destroyed(std::uint32_t entity) override BSP_UNIT_DAMAGE_UNREACHED
    std::uint32_t entity_subobject_owner(std::uint32_t entity) override BSP_UNIT_DAMAGE_UNREACHED
    float entity_subobject_lockout(std::uint32_t entity) override BSP_UNIT_DAMAGE_UNREACHED
    std::uint32_t entity_class_descriptor(std::uint32_t entity) override BSP_UNIT_DAMAGE_UNREACHED
    int entity_death_cause(std::uint32_t entity) override BSP_UNIT_DAMAGE_UNREACHED
    int last_replicated_health_byte(std::uint32_t entity) override BSP_UNIT_DAMAGE_UNREACHED
    void write_health(std::uint32_t entity, float health) override BSP_UNIT_DAMAGE_UNREACHED
    void write_full_health_marker(std::uint32_t entity) override BSP_UNIT_DAMAGE_UNREACHED
    void write_replicated_health_byte(std::uint32_t entity, int byte) override BSP_UNIT_DAMAGE_UNREACHED
    void write_invincibility(std::uint32_t entity, float value) override BSP_UNIT_DAMAGE_UNREACHED
    void write_killed_flag(std::uint32_t entity) override BSP_UNIT_DAMAGE_UNREACHED
    void write_destroyed_flag(std::uint32_t entity) override BSP_UNIT_DAMAGE_UNREACHED
    void write_death_cause(std::uint32_t entity, int cause) override BSP_UNIT_DAMAGE_UNREACHED
    UnitSessionMode session_mode() override BSP_UNIT_DAMAGE_UNREACHED
    std::size_t difficulty_level() override BSP_UNIT_DAMAGE_UNREACHED
    bool is_local_player_role(std::uint32_t entity) override BSP_UNIT_DAMAGE_UNREACHED
    bool is_player_party_unit(std::uint32_t entity) override BSP_UNIT_DAMAGE_UNREACHED
    float unit_damage_multiplier(std::size_t level) override BSP_UNIT_DAMAGE_UNREACHED
    float player_damage_multiplier(std::size_t level) override BSP_UNIT_DAMAGE_UNREACHED
    void dispatch_add_damage(std::uint32_t entity, float amount) override BSP_UNIT_DAMAGE_UNREACHED
    void dispatch_set_invincible(std::uint32_t entity, float value) override BSP_UNIT_DAMAGE_UNREACHED
    void dispatch_health_changed(std::uint32_t entity) override BSP_UNIT_DAMAGE_UNREACHED
    void dispatch_destroy(std::uint32_t entity, int recurse) override BSP_UNIT_DAMAGE_UNREACHED
    void dispatch_kill(std::uint32_t entity, int cause) override BSP_UNIT_DAMAGE_UNREACHED
    void dispatch_on_killed(std::uint32_t entity) override BSP_UNIT_DAMAGE_UNREACHED
    void dispatch_pending_destroy(std::uint32_t entity) override BSP_UNIT_DAMAGE_UNREACHED
    bool class_id_test(std::uint32_t entity, int class_id) override BSP_UNIT_DAMAGE_UNREACHED
    std::uint32_t first_child(std::uint32_t entity) override BSP_UNIT_DAMAGE_UNREACHED
    std::uint32_t next_sibling(std::uint32_t entity) override BSP_UNIT_DAMAGE_UNREACHED
    std::uint32_t hierarchy_parent(std::uint32_t entity) override BSP_UNIT_DAMAGE_UNREACHED
    bool destroy_child_predicate(std::uint32_t child, std::uint32_t parent) override BSP_UNIT_DAMAGE_UNREACHED
    bool entity_failure_latched(std::uint32_t entity) override BSP_UNIT_DAMAGE_UNREACHED
    std::uint32_t entity_word_358(std::uint32_t entity) override BSP_UNIT_DAMAGE_UNREACHED
    void write_subobject_lockout(std::uint32_t entity, float value) override BSP_UNIT_DAMAGE_UNREACHED
    int last_damage_state_index_0072ab10(std::uint32_t entity) override BSP_UNIT_DAMAGE_UNREACHED
    void dispatch_damage_state(std::uint32_t entity, int index) override BSP_UNIT_DAMAGE_UNREACHED
    bool dispatch_class_flag(std::uint32_t entity, int kind) override BSP_UNIT_DAMAGE_UNREACHED
    void dispatch_named_state_194(std::uint32_t entity, const char* name, int arg) override BSP_UNIT_DAMAGE_UNREACHED
    void dispatch_named_state_19c(std::uint32_t entity, const char* name, int arg) override BSP_UNIT_DAMAGE_UNREACHED
    float descriptor_float(std::uint32_t descriptor, std::uint32_t offset) override BSP_UNIT_DAMAGE_UNREACHED
    bool descriptor_byte_174(std::uint32_t descriptor) override BSP_UNIT_DAMAGE_UNREACHED
    void dispatch_owner_float_1ac(std::uint32_t owner, float value) override BSP_UNIT_DAMAGE_UNREACHED
    void queue_pending_destroy(std::uint32_t entity) override BSP_UNIT_DAMAGE_UNREACHED
    void queue_pending_kill(std::uint32_t entity) override BSP_UNIT_DAMAGE_UNREACHED
    void enter_entity_lock() override BSP_UNIT_DAMAGE_UNREACHED
    void leave_entity_lock() override BSP_UNIT_DAMAGE_UNREACHED
    void telemetry_event(std::uint32_t entity, const char* kind, float old_fraction, float new_fraction) override BSP_UNIT_DAMAGE_UNREACHED
    void send_health_message(std::uint32_t entity, int health_byte) override BSP_UNIT_DAMAGE_UNREACHED
    void send_death_message(std::uint32_t entity, int message_id) override BSP_UNIT_DAMAGE_UNREACHED
    void send_breakup_message(std::uint32_t entity) override BSP_UNIT_DAMAGE_UNREACHED
    bool breakup_roll(std::uint32_t entity) override BSP_UNIT_DAMAGE_UNREACHED
    float descriptor_death_selector(std::uint32_t descriptor) override BSP_UNIT_DAMAGE_UNREACHED
    float descriptor_half_width(std::uint32_t descriptor) override BSP_UNIT_DAMAGE_UNREACHED
    float descriptor_half_length(std::uint32_t descriptor) override BSP_UNIT_DAMAGE_UNREACHED
    void send_destroy_message(std::uint32_t entity, int cause, int recurse) override BSP_UNIT_DAMAGE_UNREACHED
    bool destroy_broadcast_enabled() override BSP_UNIT_DAMAGE_UNREACHED
    bool telemetry_enabled() override BSP_UNIT_DAMAGE_UNREACHED
    void clear_sink_fields(std::uint32_t entity) override BSP_UNIT_DAMAGE_UNREACHED
    void release_sink_attachment(std::uint32_t entity) override BSP_UNIT_DAMAGE_UNREACHED
    std::uint32_t parts_object(std::uint32_t entity) override BSP_UNIT_DAMAGE_UNREACHED
    std::size_t part_count(std::uint32_t parts) override BSP_UNIT_DAMAGE_UNREACHED
    float part_health(std::uint32_t parts, std::size_t index) override BSP_UNIT_DAMAGE_UNREACHED
    void write_part_health(std::uint32_t parts, std::size_t index, float value) override BSP_UNIT_DAMAGE_UNREACHED
    void detach_part(std::uint32_t parts, std::size_t index) override BSP_UNIT_DAMAGE_UNREACHED
    std::size_t group_member_count(std::uint32_t group) override BSP_UNIT_DAMAGE_UNREACHED
    std::uint32_t group_member(std::uint32_t group, std::size_t index) override BSP_UNIT_DAMAGE_UNREACHED
    std::size_t group_vector_size(std::uint32_t group) override BSP_UNIT_DAMAGE_UNREACHED
    std::uint32_t group_vector_member(std::uint32_t group, std::size_t index) override BSP_UNIT_DAMAGE_UNREACHED
    void clear_group_vector_member(std::uint32_t group, std::size_t index) override BSP_UNIT_DAMAGE_UNREACHED
    NativeUnitHealthSetterBinding bind_health_setter_00877b90(std::uint32_t entity) noexcept override BSP_UNIT_DAMAGE_UNREACHED
};
#pragma warning(pop)
#undef BSP_UNIT_DAMAGE_UNREACHED

}  // namespace bsp
