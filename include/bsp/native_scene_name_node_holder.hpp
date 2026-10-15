#pragma once

namespace bsp {
namespace game {
class GameNativeStringProcess;
class GameNativePhysicalPoolProcess;
}

// New ordinary Win32 Source APIs for the complete 58-byte bodies. The holder
// must be an accessible aligned DWORD slot; a null stored node is a no-op.
// Nonnull nodes must be genuine unique detached 14h allocations from the exact
// canonical initialized pool (E17578 for 3D80, E17540 for 3DC0). Callers settle
// mapped payloads and external links first. The same retained canonical string
// raw context, physical owner, page generation and common E188B4 list stay live.
// Externally serialize; valid disjoint stable successful nonthrowing services,
// no alias, reentry, concurrent trim or owner teardown. These functions release
// key storage, return the node, then clear the holder. They do not clear node
// headers/payloads or establish class, refcount, startup, EH or Original ABI.
void release_native_enum_symbol_node_holder_008f3d80(void* actual_holder_slot,
    game::GameNativeStringProcess& actual_strings,
    game::GameNativePhysicalPoolProcess& actual_physical);
void release_native_property_library_node_holder_008f3dc0(void* actual_holder_slot,
    game::GameNativeStringProcess& actual_strings,
    game::GameNativePhysicalPoolProcess& actual_physical);
} // namespace bsp
