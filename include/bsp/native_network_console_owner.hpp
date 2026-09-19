#pragma once
#include <cstddef>
#include <cstdint>

namespace bsp {
// Actual allocation size at73DC9D. The base lifetime writes only the primary
// profile and singleton publication. Derived arrays, locks and two workers
// belong to A3D060/A3D1A0 and are not constructed by these base bodies.
struct alignas(4) NativeNetworkConsoleStorage { std::byte bytes[0x4003f0]; };
struct NativeNetworkConsoleBaseContext {
    NativeNetworkConsoleStorage* volatile& current_manager_00f8abdc;
    void* volatile& singleton_manager_01090aa0;
};
// ECX actual owner, EAX same owner, RET. Publish before the second manager
// getter and register CURRENT F8ABDC under the first captured tracked lock.
NativeNetworkConsoleStorage* construct_native_network_console_base_00a3cf00(
    NativeNetworkConsoleStorage&, NativeNetworkConsoleBaseContext&);
// ECX captured owner, RET. Unregister CURRENT F8ABDC, clear publication, leave
// the captured section and restore CE3818 on the captured owner.
void destroy_native_network_console_base_00a3cfa0(
    NativeNetworkConsoleStorage&, NativeNetworkConsoleBaseContext&);
// ECX captured owner, stack flags, RET4/EAX captured identity. Only bit0 frees.
// Flags1 requires `new NativeNetworkConsoleStorage` (no value initialization).
// These source interfaces preserve the existing tracked-guard C++ cleanup;
// native private EH/SEH and binary ABI equivalence remain unproved.
NativeNetworkConsoleStorage* delete_native_network_console_base_00a3d040(
    NativeNetworkConsoleStorage&, std::uint32_t flags, NativeNetworkConsoleBaseContext&);
} // namespace bsp
