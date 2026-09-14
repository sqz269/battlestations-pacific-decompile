#include "bsp/native_unit_class_lua.hpp"
#include "bsp/native_lua_class_setters.hpp"
#include "bsp/native_mission_entity_lua_self.hpp"
#include <cstring>
#include <new>

namespace bsp {
namespace {
// FH3 mapDDA634: 0->-1 self,1->0 ClassID,2->0 Class,3->2 globals,
// 4->3 VehicleClass,5->4 row. Headers have no implicit destructor cleanup.
struct Cleanup final {
    NativeLuaObjectStorage self, globals, classes, row;
    NativeString key;
    NativeStringRawPoolContext& strings;
    int state = -1;
    explicit Cleanup(NativeStringRawPoolContext& context) : strings(context) {}
    void step() {
        switch (state) {
        case 5: state = 4; destroy_native_lua_object_00b67700(row); break;
        case 4: state = 3; destroy_native_lua_object_00b67700(classes); break;
        case 3: state = 2; destroy_native_lua_object_00b67700(globals); break;
        case 2: case 1:
            state = 0; destroy_native_string_header_0041dd20(&key, strings); break;
        case 0: state = -1; destroy_native_lua_object_00b67700(self); break;
        default: break;
        }
    }
    ~Cleanup() noexcept(false) { while (state >= 0) step(); }
    void construct_key(const char* literal, std::uint32_t length) {
        // Reconstruct the same actual8h header; a prior release leaves stale
        // fields which native explicitly resets before the second resize.
        key.~NativeString();
        ::new (&key) NativeString;
        resize_native_string_header_0041dd40(&key, strings, length, true);
        char* const data = key.data();
        if (data) std::memcpy(data, literal, key.length() + 1u);
    }
};
} // namespace

void bind_native_unit_class_lua_009292b0(
    const NativeString& actual_entity_key_178, std::int32_t class_id,
    const char* name, void* volatile& actual_world_publication_00e188a8,
    NativeStringRawPoolContext& strings) {
    Cleanup locals(strings);
    construct_native_mission_entity_lua_self_00927b40(
        actual_entity_key_178, locals.self, actual_world_publication_00e188a8);
    locals.state = 0;
    locals.construct_key("ClassID", 7);
    locals.state = 1;
    native_lua_set_integer_00b67460(locals.self, locals.key, class_id);
    locals.step();
    native_lua_set_cstring_00b66790(locals.self, "Name", name);
    locals.construct_key("Class", 5);
    void* const world = actual_world_publication_00e188a8;
    auto& owner = *reinterpret_cast<NativeLuaStateStorage*>(
        static_cast<std::byte*>(world) + 0x1a0c);
    locals.state = 2;
    auto* const globals = native_lua_globals_00b67980(owner, &locals.globals);
    locals.state = 3;
    auto* const classes = native_lua_get_by_name_00b67800(
        *globals, &locals.classes, "VehicleClass");
    locals.state = 4;
    auto* const row = native_lua_get_by_index_00b67720(*classes, &locals.row, class_id);
    locals.state = 5;
    native_lua_set_object_00b675d0(locals.self, locals.key, *row);
    while (locals.state >= 0) locals.step();
}
} // namespace bsp
