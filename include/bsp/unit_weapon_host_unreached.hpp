#pragma once
// A base for host adapters over bsp::UnitWeaponHost (include/bsp/unit_weapons.hpp).
//
// UnitWeaponHost carries the engine services of the unit-weapon Lua bindings as pure
// virtuals, and no game host implemented it, so the bindings that take it never ran.
// This class overrides every one of them (TorpedoStockHost's included) with a loud
// failure; an adapter derives from it and overrides only the services its own path
// reaches, each mapped onto the host's real objects. A service the adapter does not
// override throws std::logic_error naming the service, so a path the adapter did not
// read fails at once instead of running on an invented value.
//
// Written mechanically from the declarations in unit_weapons.hpp (goal 5 chain 660), one
// override per pure virtual, same signature, in the header's order, as
// bsp/left55_seg56a_host_unreached.hpp does for Left55Seg56aHost. A C++ adapter base, not
// an image type: no address, no image layout. Keep it in step with UnitWeaponHost.

#include "bsp/unit_weapons.hpp"

#include <stdexcept>
#include <string>

namespace bsp {

[[noreturn]] inline void unit_weapon_host_unreached(const char* service) {
    throw std::logic_error(std::string("UnitWeaponHost service not provided by this adapter: ")
        + service);
}

// One macro body, so nothing reading the sources for implementations of a
// service (tools/wire_readiness.py among them) takes one for an implementation.
#define BSP_UNIT_WEAPON_UNREACHED { ::bsp::unit_weapon_host_unreached(__func__); }
#pragma warning(push)
#pragma warning(disable : 4100)  // the parameter names document the services
class UnitWeaponHostUnreached : public UnitWeaponHost {
public:
    NativeHandle entity_from_lua_argument(int index) override BSP_UNIT_WEAPON_UNREACHED
    NativeHandle entity_from_lua_ptr_field(int index) override BSP_UNIT_WEAPON_UNREACHED
    int lua_argument_int(int index) override BSP_UNIT_WEAPON_UNREACHED
    bool lua_argument_bool(int index) override BSP_UNIT_WEAPON_UNREACHED
    float lua_argument_number(int index) override BSP_UNIT_WEAPON_UNREACHED
    const char* lua_argument_string(int index) override BSP_UNIT_WEAPON_UNREACHED
    bool lua_argument_is_nil(int index) override BSP_UNIT_WEAPON_UNREACHED
    bool lua_argument_is_entity(int index) override BSP_UNIT_WEAPON_UNREACHED
    void lua_argument_vector3(int index, float out[3]) override BSP_UNIT_WEAPON_UNREACHED
    int lua_argument_count() override BSP_UNIT_WEAPON_UNREACHED
    void lua_push_boolean(bool value) override BSP_UNIT_WEAPON_UNREACHED
    void lua_push_number(float value) override BSP_UNIT_WEAPON_UNREACHED
    void lua_push_nil() override BSP_UNIT_WEAPON_UNREACHED
    void lua_push_entity(NativeHandle entity) override BSP_UNIT_WEAPON_UNREACHED
    void lua_new_result_table() override BSP_UNIT_WEAPON_UNREACHED
    void lua_result_table_set(int key, NativeHandle entity) override BSP_UNIT_WEAPON_UNREACHED
    int lua_result_count() override BSP_UNIT_WEAPON_UNREACHED
    bool class_test(NativeHandle object, int class_id) override BSP_UNIT_WEAPON_UNREACHED
    NativeHandle weapon_director(NativeHandle unit) override BSP_UNIT_WEAPON_UNREACHED
    int device_root_count(NativeHandle unit) override BSP_UNIT_WEAPON_UNREACHED
    NativeHandle device_root(NativeHandle unit, int slot) override BSP_UNIT_WEAPON_UNREACHED
    NativeHandle node_first_child(NativeHandle node) override BSP_UNIT_WEAPON_UNREACHED
    NativeHandle node_next_sibling(NativeHandle node) override BSP_UNIT_WEAPON_UNREACHED
    int gun_barrel_num(NativeHandle gun) override BSP_UNIT_WEAPON_UNREACHED
    int gun_next_fire_barrel(NativeHandle gun) override BSP_UNIT_WEAPON_UNREACHED
    float gun_reload_timer(NativeHandle gun, int barrel) override BSP_UNIT_WEAPON_UNREACHED
    float gun_throw_a(NativeHandle gun) override BSP_UNIT_WEAPON_UNREACHED
    float gun_throw_b(NativeHandle gun) override BSP_UNIT_WEAPON_UNREACHED
    int gun_weapon_type(NativeHandle gun) override BSP_UNIT_WEAPON_UNREACHED
    float gun_reload_empty_threshold() override BSP_UNIT_WEAPON_UNREACHED
    void gun_set_reload_timer(NativeHandle gun, int barrel, float time, bool set_full) override BSP_UNIT_WEAPON_UNREACHED
    void gun_rearm_barrel(NativeHandle gun, int barrel, bool full) override BSP_UNIT_WEAPON_UNREACHED
    void gun_fire(NativeHandle gun, int use_explicit_throw, float throw_a, float throw_b) override BSP_UNIT_WEAPON_UNREACHED
    bool director_stance_allows_fire(NativeHandle director, int stance) override BSP_UNIT_WEAPON_UNREACHED
    bool director_stance_allows_move(NativeHandle director, int stance) override BSP_UNIT_WEAPON_UNREACHED
    void director_apply_fire_permission(NativeHandle director, bool allow) override BSP_UNIT_WEAPON_UNREACHED
    void director_apply_move_permission(NativeHandle director, bool allow) override BSP_UNIT_WEAPON_UNREACHED
    NativeHandle director_fire_target(NativeHandle director) override BSP_UNIT_WEAPON_UNREACHED
    void director_clear_fire_target(NativeHandle director) override BSP_UNIT_WEAPON_UNREACHED
    void director_set_fire_target_entity(NativeHandle director, NativeHandle target) override BSP_UNIT_WEAPON_UNREACHED
    void director_set_fire_target_position(NativeHandle director, const float position[3]) override BSP_UNIT_WEAPON_UNREACHED
    NativeHandle unit_secondary_owner(NativeHandle unit) override BSP_UNIT_WEAPON_UNREACHED
    int weapon_name_index_006e8250(const char* weapon_name) override BSP_UNIT_WEAPON_UNREACHED
    float director_fire_stamp(NativeHandle director, int index) override BSP_UNIT_WEAPON_UNREACHED
    float world_clock_00f876a4() override BSP_UNIT_WEAPON_UNREACHED
    double lua_argument_number_f64(int index) override BSP_UNIT_WEAPON_UNREACHED
    int loaded_torpedo_barrels_00810e90(NativeHandle unit) override BSP_UNIT_WEAPON_UNREACHED
    void unload_random_torpedo_barrel_0081dcb0(NativeHandle unit) override BSP_UNIT_WEAPON_UNREACHED
    void unit_set_torpedo_spare(NativeHandle unit, int spare) override BSP_UNIT_WEAPON_UNREACHED
    int live_torpedo_count() override BSP_UNIT_WEAPON_UNREACHED
    NativeHandle live_torpedo_at(int index) override BSP_UNIT_WEAPON_UNREACHED
    bool torpedo_is_dead(NativeHandle torpedo) override BSP_UNIT_WEAPON_UNREACHED
    NativeHandle torpedo_owner(NativeHandle torpedo) override BSP_UNIT_WEAPON_UNREACHED
    void route_weapon_enable(const WeaponEnableMessage& message) override BSP_UNIT_WEAPON_UNREACHED
};
#pragma warning(pop)
#undef BSP_UNIT_WEAPON_UNREACHED

}  // namespace bsp
