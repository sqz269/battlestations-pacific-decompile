#pragma once

namespace bsp {
struct NativeStringRawPoolContext;

// Whole119B/38op normal control flow of00469010. Native ECX=actual14h
// record, unused EDX, plain RET/semantic void. Explicit saved-this4 supplies
// only the cell used by the separately qualified cleanup action. Binding
// metadata/context stays stable and disjoint from all reached raw accesses;
// record/cell payloads retain their actual read/write/alias schedules.
// No implicit compiler/private-FH3/outgoing-argument homes are supplied.
// Actual pool publications/gate/manager and their existing provider contracts
// must remain valid. Ordinary C++ cleanup/current CRT transport is qualified:
// no Original heap, FH3/private frame, binary ABI, dispatch or game equivalence.
void destroy_native_scene_command_record_00469010(void* actual_record,
    void* actual_saved_this4, NativeStringRawPoolContext& strings);

// Qualified whole11B/3op action00C61A50. Reread CURRENT saved-this4, add4
// with x86 word arithmetic and destroy that current actual pooled header.
void unwind_native_scene_command_record_00c61a50(const void* actual_saved_this4,
    NativeStringRawPoolContext& strings);

struct NativeSceneCommandRecordScalarDeleteFrame {
    const void* actual_flags4;
    void* actual_destructor_saved_this4;
};

// Whole30B/11op00469B00. Native ECX record/unusedEDX/stackflags4/RET4.
// Read only the current flags BYTE after child return; mask01 means bit0.
// Return the captured raw record value, including after its conditional free;
// this does not promise a live object. Frame metadata is stable/disjoint and
// the actual flags/saved-this backing is explicit, without private ABI credit.
void* scalar_delete_native_scene_command_record_00469b00(void* actual_record,
    const NativeSceneCommandRecordScalarDeleteFrame& frame,
    NativeStringRawPoolContext& strings);

// Whole35B/15op0046A090. Native ECX actual pointer-cell/unusedEDX/plainRET.
// Capture slot and its record once. If nonnull, destroy/free that captured
// record and only then zero that captured slot. No pointee/owner reload.
// The reached final slot write must remain valid under current CRT transport.
void delete_native_scene_command_record_pointer_0046a090(void* actual_slot4,
    void* actual_destructor_saved_this4, NativeStringRawPoolContext& strings);
} // namespace bsp
