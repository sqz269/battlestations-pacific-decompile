#include "bsp/native_lua_service_bindings.hpp"
#include "bsp/native_lua_script_overrides.hpp"
#include <stdexcept>

namespace bsp {
namespace {
thread_local NativeLuaServiceBindings* active_binding = nullptr;
}

NativeLuaServiceBindings::NativeLuaServiceBindings(NativeLuaFundamentalsContext& fundamentals,
    const volatile std::uint8_t& x360comp, const NativeString& region) noexcept
    : fundamentals_(fundamentals),
      files_{fundamentals.manager_0109ceec, &append_overrides, &do_file,
          fundamentals.vfs},
      bootstrap_{x360comp, region, &fundamentals,
          &native_lua_fundamentals_callback, &do_file} {}

NativeLuaServiceBindings::Activation::Activation(NativeLuaServiceBindings& binding) noexcept
    : previous_(active_binding) {
    active_binding = &binding;
}
NativeLuaServiceBindings::Activation::~Activation() noexcept {
    active_binding = previous_;
}

NativeLuaServiceBindings& NativeLuaServiceBindings::active() {
    if (!active_binding)
        throw std::logic_error("native Lua callback invoked without active source service binding");
    return *active_binding;
}
int NativeLuaServiceBindings::do_file(lua_State* state) {
    auto& binding = active();
    return do_native_lua_file_00b69e00(state,binding.fundamentals_.strings,binding.files_);
}
void NativeLuaServiceBindings::append_overrides(void* manager,
    const NativeString& path, NativeStringVectorStorage& output) {
    auto& binding = active();
    append_native_lua_script_overrides_00bdef90(manager,path,output,
        binding.fundamentals_.strings,binding.fundamentals_.vfs);
}
} // namespace bsp
