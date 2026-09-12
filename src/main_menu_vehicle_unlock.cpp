#include "bsp/main_menu_vehicle_unlock.hpp"

#include "bsp/native_lua_objects.hpp"
#include "bsp/native_string_compare.hpp"
#include "bsp/profile_reset.hpp"

#include <string_view>

namespace bsp {
namespace {

// Only lifetime guards: all construction, assignment, registration and release
// use the shared bodies over the actual address-stable LuaObject storage.
class LuaCleanup final {
public:
    explicit LuaCleanup(NativeLuaObjectStorage& object) noexcept : object_(&object) {}
    ~LuaCleanup() { if (object_) destroy_native_lua_object_00b67700(*object_); }
    void release() {
        auto* const object = object_;
        object_ = nullptr;
        destroy_native_lua_object_00b67700(*object);
    }
    LuaCleanup(const LuaCleanup&) = delete;
    LuaCleanup& operator=(const LuaCleanup&) = delete;
private:
    NativeLuaObjectStorage* object_;
};

class StringCleanup final {
public:
    StringCleanup(NativeString& string, NativeStringStorage& storage) noexcept
        : string_(string), storage_(storage) {}
    ~StringCleanup() { destroy_native_string_header_0041dd20(&string_, storage_); }
    StringCleanup(const StringCleanup&) = delete;
    StringCleanup& operator=(const StringCleanup&) = delete;
private:
    NativeString& string_;
    NativeStringStorage& storage_;
};

} // namespace

bool main_menu_vehicle_class_unlocked_00584750(bool first_argument,
    std::int32_t vehicle_class, MainMenuVehicleUnlockStorage& storage,
    NativeStringStorage& strings) {
    (void)first_argument;
    if (vehicle_class == -1) return true;

    NativeLuaObjectStorage globals;
    native_lua_globals_00b67980(storage.lua_owner_00e188a8_1a0c(), &globals);
    LuaCleanup globals_cleanup(globals);
    NativeLuaObjectStorage classes;
    native_lua_get_by_name_00b67800(globals, &classes, "VehicleClass");
    LuaCleanup classes_cleanup(classes);
    globals_cleanup.release();

    // The otherwise unused default object at native ESP+70 is still created
    // and destroyed in order; assignment does not replace that constructor.
    NativeLuaObjectStorage unused;
    construct_native_lua_object_00b65f50(&unused);
    LuaCleanup unused_cleanup(unused);
    NativeLuaObjectStorage entry;
    construct_native_lua_object_00b65f50(&entry);
    LuaCleanup entry_cleanup(entry);
    {
        NativeLuaObjectStorage indexed;
        native_lua_get_by_index_00b67720(classes, &indexed, vehicle_class);
        LuaCleanup indexed_cleanup(indexed);
        assign_native_lua_object_00b67690(entry, indexed);
    }
    if (!native_lua_is_table_00b661b0(entry)) return false;

    NativeLuaObjectStorage unlock;
    native_lua_get_by_name_00b67800(entry, &unlock, "Unlock");
    LuaCleanup unlock_cleanup(unlock);
    if (!native_lua_is_boolean_00b66000(unlock)) return false;
    if (!native_lua_boolean_00b66250(unlock)) return false;

    NativeLuaObjectStorage id;
    native_lua_get_by_name_00b67800(entry, &id, "UnlockID");
    LuaCleanup id_cleanup(id);
    if (native_lua_is_nil_00b65fb0(id)) return true;
    if (!native_lua_is_string_00b660a0(id)) return false;

    NativeString comparison;
    comparison.assign_0041e870(strings, native_lua_string_00b662b0(id));
    StringCleanup comparison_cleanup(comparison, strings);
    if (equal_native_string_header_00425850(&comparison, "")) return true;

    // Preserve the second getter and separate allocation. Allocation may
    // change the current global game: the profile read follows it at5848EA.
    NativeString expression;
    expression.assign_0041e870(strings, native_lua_string_00b662b0(id));
    StringCleanup expression_cleanup(expression, strings);
    const auto& profile = storage.profile_00e188a8_650();
    const char* const text = expression.data();
    return is_unlock_expression_satisfied_007fc4c0(
        profile.unlock_state, std::string_view(text ? text : ""));
}

} // namespace bsp
