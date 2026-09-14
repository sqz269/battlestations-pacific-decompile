#include "bsp/native_system_registry_raw_terminal.hpp"

#include <exception>

namespace bsp {
namespace {
struct BaseCleanup {
    NativeSystemConstantRegistryStorage& owner;
    NativeSystemConstantRegistryRawContext& context;
    bool armed=true;
    ~BaseCleanup() noexcept {
        if(armed) {
            try {destroy_native_system_constant_base_00b5ba80(owner,context);}
            catch(...) {std::terminate();}
        }
    }
};
} // namespace

void destroy_native_system_constant_registry_00b5df00(
    NativeSystemConstantRegistryStorage& owner,NativeSystemConstantRegistryRawContext& context) {
    owner.vtable_00=0x00d62a3c; // B5DF1E.
    BaseCleanup cleanup{owner,context}; // B5DF2B: state0 -> CC1120 -> B5BA80.
    destroy_native_system_constant_array_00b5bf50(owner.constants_04,context.strings);
    cleanup.armed=false; // B5DF45: state-1 BEFORE the normal base call.
    destroy_native_system_constant_base_00b5ba80(owner,context);
}

NativeSystemConstantRegistryStorage* delete_native_system_constant_registry_00b5df70(
    NativeSystemConstantRegistryStorage* owner,NativeSystemConstantRegistryRawContext& context,
    std::uint32_t flags) {
    auto* const captured=owner;
    destroy_native_system_constant_registry_00b5df00(*captured,context);
    if(flags&1u)singleton_lifetime_free(captured);
    return captured;
}

NativeSystemConstantRegistryStorage* delete_native_system_constant_base_00b5bb20(
    NativeSystemConstantRegistryStorage* owner,NativeSystemConstantRegistryRawContext& context,
    std::uint32_t flags) {
    auto* const captured=owner;
    destroy_native_system_constant_base_00b5ba80(*captured,context);
    if(flags&1u)singleton_lifetime_free(captured);
    return captured;
}
} // namespace bsp
