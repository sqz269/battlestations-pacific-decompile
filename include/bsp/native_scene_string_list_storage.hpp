#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual scene string-list storage requires MSVC Win32.
#endif

namespace bsp {

// Whole 0094EFB0 [0094EFB0,0094EFE1), 49B/18 instructions. Native ECX is
// the caller's actual aligned writable raw12 header: slots+0, count+4,
// capacity+8. Store capacity64 before exact256-byte allocation; publish slots,
// then count0, and return the same header. The allocated slots stay uninitialized.
// No stacked inputs; plain RET. No host class, vtable or container is created.
void* __fastcall construct_native_scene_string_list_storage_0094efb0(
    void* actual_header12);

// Whole 0094F010 [0094F010,0094F061), 81B/34 instructions. Native ECX is the
// same actual raw12; no stacked inputs, plain RET, no semantic EAX result.
// Publish capacity+64 before allocation, retaining native MUL4 saturation.
// After allocation capture CURRENT slots/count; call current memcpy(count*4),
// including zero; reload CURRENT slots after copy, conditionally free/clear0,
// then publish the captured new slots. Count stays untouched. Slots are opaque.
void __fastcall grow_native_scene_string_list_storage_0094f010(
    void* actual_header12);

// Admitted Source domain: ordinary successful positive allocatable nonwrapping
// sizes; genuine constructor/earlier accepted-growth storage from the matching
// current heap; coherent 0<=count<=oldcapacity and oldcapacity<=3FFFFFBFh;
// valid initialized reached slot bytes; header/old/new/private frame disjoint;
// flat ES, DF clear, serialized operation without faults, throws or reentry.
// Empty growth still copies zero bytes between real nonnull allocations.
// Retained overflow instructions do not admit overflow/failure behavior.
//
// Current allocator/free/CRT bindings are explicit. This new Source interface
// does not admit Original private CRT/EH, complete ABI/fault identity, populated
// caller ownership, append/clear/retirement, application startup or game proof.
// Pending 0094F270,0094F3E0,0094F540 and the 008F3270 create/pass/retire binding
// remain separate. The helpers neither inspect nor retire any element.

} // namespace bsp
