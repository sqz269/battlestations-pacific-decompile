#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw scene-record destruction requires MSVC Win32.
#endif

namespace bsp {
struct NativeStringRawPoolContext;

// Stable/disjoint Source binding metadata. At destructor entry Q, saved_this4
// binds the actual private [EBP-10h] cell at Q-10h. Both native stores occur
// before the vtable publication. Payload may alias this actual cell when all
// reached accesses stay valid; implicit saved-register/FH3/spill aliases are not
// supplied. This is not a native object or a compiler-frame simulation.
void destroy_native_scene_hidden_entity_record_00469300(void* actual_record,
    void* actual_saved_this4, NativeStringRawPoolContext&);

// Whole private cleanup action semantics only: read CURRENT saved_this4,
// add54 modulo32, and use the real raw string-header destructor. This qualified
// Source guard mapping does not establish Original FH3/SEH transport.
void unwind_native_scene_hidden_entity_record_00c61ab0(
    const void* actual_saved_this4, NativeStringRawPoolContext&);

struct NativeSceneHiddenRecordDeleteFrame {
    const void* actual_flags4;
    void* actual_child_saved_this4;
};

// At wrapper entry P, flags4=P+4 and child saved_this4=P-18h are distinct
// actual regions. Capture record bits, invoke the complete raw destructor,
// THEN read only the live flag BYTE and conditionally use the matching actual
// host CRT free service. Return the captured DWORD bits even after release;
// they carry no surviving-object/lifetime/ownership promise.
std::uint32_t delete_native_scene_hidden_entity_record_004693a0(
    void* actual_record, const NativeSceneHiddenRecordDeleteFrame&,
    NativeStringRawPoolContext&);

// GENERAL SOURCE-READY IS FALSE. Whole control-flow Source uses an actual raw
// bag vtable slot0 call (__thiscall, this=bag, stacked1; ignored result), never
// an injected callback or synthetic vtable. The reached slot must be callable
// under that ABI. Header/payload/frame bindings and current CRT heap releases
// must be valid. This does not recover every dynamic target or Original CRT
// heap/handler/failure behavior. Current C++ guard/host service transport is
// qualified; Original argument homes/register/FH3/fault/binary ABI, constructed
// Source-vtable dispatch compatibility and game execution remain unproved.
}
