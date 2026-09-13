#include "bsp/native_mission_entity_lua_self.hpp"

#include <cstddef>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native mission entity Lua self requires MSVC Win32.
#endif

namespace bsp {
namespace {
// Same embedded owner offset as kMissionLuaStateOwnerField. The actual world
// constructor004DDE8A/004DDE9B produces it with canonical00B66BD0.
constexpr std::size_t world_lua_owner_offset = 0x1a0c;

class CompletedObject final {
public:
    explicit CompletedObject(NativeLuaObjectStorage& object) noexcept : object_(object) {}
    ~CompletedObject() { if (active_) destroy_native_lua_object_00b67700(object_); }
    void arm() noexcept { active_ = true; }
    void dismiss() noexcept { active_ = false; }
    void destroy() {
        active_ = false; // native state transitions BEFORE the destructor call
        destroy_native_lua_object_00b67700(object_);
    }
private:
    NativeLuaObjectStorage& object_;
    bool active_ = false;
};
} // namespace

NativeLuaObjectStorage* construct_native_mission_entity_lua_self_00927b40(
    const NativeString& actual_entity_key_178,
    NativeLuaObjectStorage& fresh_output,
    void* volatile& actual_world_publication_00e188a8) {
    CompletedObject output_cleanup(fresh_output); // native state0 flag initially0
    NativeLuaObjectStorage globals;
    CompletedObject globals_cleanup(globals);
    NativeLuaObjectStorage self_table;
    CompletedObject table_cleanup(self_table);

    void* const world = actual_world_publication_00e188a8;
    auto& lua_owner = *reinterpret_cast<NativeLuaStateStorage*>(
        static_cast<std::byte*>(world) + world_lua_owner_offset);
    auto* const global_result = native_lua_globals_00b67980(lua_owner, &globals);
    globals_cleanup.arm(); // native state1; state0 output flag still0
    auto* const table_result = native_lua_get_by_name_00b67800(
        *global_result, &self_table, "thisTable");
    table_cleanup.arm(); // native state2
    native_lua_get_by_string_00b68100(*table_result, &fresh_output, actual_entity_key_178);
    output_cleanup.arm(); // caller's output constructed; native state0 flag1
    table_cleanup.destroy(); // state2 ->1 before B67700, can move output's index
    globals_cleanup.destroy(); // state1 ->0 before B67700
    output_cleanup.dismiss();
    return &fresh_output;
}
} // namespace bsp
