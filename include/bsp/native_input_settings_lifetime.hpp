#pragma once
#include "bsp/native_input_settings_tables.hpp"

namespace bsp {
// Required library boundaries on actual native headers, iterators and rows.
// They destroy populated storage; no empty-container/default implementation.
struct NativeInputSettingsDestructionCalls {
    virtual ~NativeInputSettingsDestructionCalls() = default;
    virtual void* call_006a7aa0(void* tree, void* output,
        NativeKeyboardTreeIterator first, NativeKeyboardTreeIterator last) = 0;
    virtual void* call_006a6a20(void* tree, void* output,
        NativeKeyboardTreeIterator first, NativeKeyboardTreeIterator last) = 0;
    virtual void* call_0069fe70(void* tree, void* output,
        NativeKeyboardTreeIterator first, NativeKeyboardTreeIterator last) = 0;
    virtual void* call_006a1aa0(void* tree, void* output,
        NativeKeyboardTreeIterator first, NativeKeyboardTreeIterator last) = 0;
};

struct NativeInputSettingsLifetimeContext {
    void* volatile& publication_00e198e8;
    void* volatile& manager_publication_01090aa0;
    NativeInputSettingsTableServices& tables;
    NativeInputSettingsDestructionCalls& containers;
};

// Full 005547D0: native no inputs, EAX, RET. Fast captured return; slow path
// captures the first raw manager section, enters/rechecks, allocates540h,
// constructs, publishes, resolves the manager again, registers the CURRENT
// publication, leaves the captured section, then reloads the return value.
void* get_native_input_settings_005547d0(NativeInputSettingsLifetimeContext&);

// Full 006AB6B0: native ECX fresh540h, EAX same, RET. Existing actual member
// construction plus complete table loader. A loader exception closes persistent
// Lua and destroys populated members before base/publication cleanup.
void* construct_native_input_settings_006ab6b0(void*, NativeInputSettingsLifetimeContext&);

// Full normal 006AA460: native ECX settings, RET. Close Lua twice, destroy
// duplicate devices/controller names/counts/conflict pairs/groups/input names/
// device order/devices; clear publication unconditionally and stamp CE3818.
// Native state5 remains armed during conflict-pair cleanup; do not silently
// replace its exceptional retry with a uniform disarm-before-call policy.
void destroy_native_input_settings_006aa460(void*, NativeInputSettingsLifetimeContext&);

// 006AB800: native ECX owner, stack flags, EAX original pointer bits, RET4.
// Destruction precedes the bit0 free test. Neither destructor unregisters.
void* scalar_delete_native_input_settings_006ab800(
    void*, std::uint32_t flags, NativeInputSettingsLifetimeContext&);

// New source service ABIs, not callable original virtual tables. Production
// container providers remain required. The raw manager's deletion bindings
// must borrow this same context through input_settings for CF81CC dispatch.
// No original FH3, private-stack aliases, hardware-fault or gameplay proof.
} // namespace bsp
