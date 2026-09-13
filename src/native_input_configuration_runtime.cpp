#include "bsp/native_input_configuration_runtime.hpp"
#include <cstddef>

namespace bsp {
namespace {
struct ParsedCleanup {
    NativeInputConfigurationParsedObjects& objects;
    unsigned remaining = 4;
    ~ParsedCleanup() noexcept { release(); }
    void release() {
        // Native normal sites lower their cleanup state before each call.
        // On a supported exception, only the still-owned objects are unwound.
        while (remaining) {
            switch (--remaining) {
            case 3: destroy_native_lua_object_00b67700(objects.value); break;
            case 2: destroy_native_lua_object_00b67700(objects.key); break;
            case 1: destroy_native_lua_object_00b67700(objects.inputs); break;
            case 0: destroy_native_lua_object_00b67700(objects.globals); break;
            }
        }
    }
};
} // namespace

void load_native_input_configuration_00698a10(
    void* configuration, NativeInputConfigurationRuntimeServices& services) {
    NativeInputConfigurationParsedObjects objects;
    load_native_input_configuration_actions_prefix_00698a10(configuration,objects,services.parsing);
    ParsedCleanup cleanup{objects};
    auto& action_context = services.parsing.loading.cleanup.action_owner;
    void* owner = get_native_input_action_owner_004bec00(action_context);
    rebind_all_native_input_actions_00a922a0(owner,services.tick.backend_00f8bbf4);
    float seconds;
    __asm { fldz }
    __asm { fstp seconds }
    owner = get_native_input_action_owner_004bec00(action_context);
    update_native_input_action_owner_00a92c40(owner,seconds,services.tick);
    if (services.parsing.loading.x360comp_00f88a30 == 0) {
        void* settings = get_native_input_settings_005547d0(services.settings);
        preserve_native_input_default_bindings_006ab820(settings,services.defaults);
    }
    static_cast<std::byte*>(configuration)[0x520] = std::byte{1};
    cleanup.release();
}
} // namespace bsp
