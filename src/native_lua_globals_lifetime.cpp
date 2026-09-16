#include "bsp/native_lua_globals_lifetime.hpp"
#include <cstdlib>
#include <stdexcept>

namespace bsp {
namespace {
NativeLuaRegionLifetimeContext* process_context{};
NativeLuaRegionLifetimeContext& context() {
    if (!process_context)
        throw std::logic_error("native Lua region process context is unbound");
    return *process_context;
}
}
void bind_static_native_lua_region_0108ff24(NativeLuaRegionLifetimeContext& value) {
    if (process_context && process_context != &value)
        throw std::logic_error("native Lua region process context cannot be replaced");
    process_context = &value;
}
int initialize_static_native_lua_region_00cd7ce0() {
    (void)context();
    return std::atexit(&destroy_static_native_lua_region_00ce0d60);
}
void destroy_native_lua_region_00ce0d60(NativeLuaRegionLifetimeContext& value) {
    destroy_native_string_header_0041dd20(&value.actual_region_0108ff24,
        value.actual_strings);
}
void destroy_static_native_lua_region_00ce0d60() {
    destroy_native_lua_region_00ce0d60(context());
}
} // namespace bsp
