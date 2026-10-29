#pragma once

namespace bsp {
struct NativeGameCurrentVirtualCalls;

// Complete normal008844F0..0088450A (27B/12 instructions). Native ECX=host,
// no stack args/plain RET; ESI preserved. This new void C++ interface does not
// reproduce native register/result/flags/fault/exception transport or ABI.
//
// actual_host is genuine readable/writable Win32 host storage, valid through
// the child callback. Only its DWORD+4 is consumed; no native host class or
// container layout is inferred. The native has no host-null guard, including
// at the unconditional OnDestroy caller. Null host is not an admitted input.
//
// Capture+4 once. A nonnull machine receives its CURRENT callable slot0(1)
// through the existing concrete dispatcher, then the same host cell is cleared
// after normal return even if the callback replaced it. Initial null machine
// gets no call or clear. Child constructor/table/code/free custody must already
// be genuine and retained. Numeric original PE vtable entries are not callable
// host pointers. No default, abstract stub, allocation or rollback is supplied.
// Existing semantic MissionLuaHostState APIs/callers remain separate.
void release_native_mission_lua_machine_008844f0(
    void* actual_host, NativeGameCurrentVirtualCalls& actual_dispatch);
} // namespace bsp
