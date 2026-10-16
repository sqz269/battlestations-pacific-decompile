#pragma once

namespace bsp {

struct NativeStringRawPoolContext;
class NativeScenePropertyCloneContext;

// Explicit bindings for the one native LIVE source-pointer argument and the
// saved-this cell read by the separate cleanup action. The binding metadata and
// both contexts stay stable/disjoint from every reached native read/write span.
// The actual cells/payloads retain their reached raw alias schedules; implicit
// compiler, outgoing-argument and Original FH3 private homes are not supplied.
struct NativeSceneHiddenRecordCopyConstructFrame {
    const void* actual_parameters4;
    void* actual_saved_this4;
};

// Whole normal control flow of 00469240,181B/58ops. Native ECX=record, one
// LIVE stack source-pointer word, RET4/EAX=captured record. This new C++ API
// exposes actual backing and concrete borrowed contexts, not Original ABI.
// The original image word CE5640 is a borrowed raw tag, not a Source vtable.
//
// General Source-ready is FALSE. Inherit BOTH actual providers' admissions:
// 008F41F0's successful canonical finite/stable/unshared/acyclic owning graph,
// key/payload/count/locale/FP domain and 00438E40's successful nonwrapping,
// readable NUL text and disjoint current allocation domain. The clone context
// and raw pool context refer to the same live canonical string process, whose
// required startup has returned, and the clone's actual physical process.
// Preserve every selected provider contract; add no validation or fallback.
// No failure/new-handler/reentry/private unwind/Original FH3, register/stack/FS,
// class dispatch, binary ABI, consumer readiness or game validation is claimed.
void* copy_construct_native_scene_hidden_entity_record_00469240(
    void* actual_record,
    const NativeSceneHiddenRecordCopyConstructFrame& frame,
    NativeStringRawPoolContext& strings,
    NativeScenePropertyCloneContext& clone_context);

// Qualified whole 11B/3op action mapping of 00C61A90: reread CURRENT saved-this,
// add54 with x86 word arithmetic and destroy only that actual pooled header.
// This supplies ordinary cleanup transport, not Original FH3/state identity.
void unwind_native_scene_hidden_entity_record_copy_00c61a90(
    const void* actual_saved_this4, NativeStringRawPoolContext& strings);

} // namespace bsp
