#pragma once

#include <cstddef>
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native scene property array assignment requires MSVC Win32.
#endif

namespace bsp {

inline constexpr std::size_t native_scene_property_array_assignment_record_bytes = 0x38;

// Four whole Original entries, descriptive names are hypotheses:
// 008F0460..008F04B6:86B33; 008F04C0..008F051A:90B35;
// 008F0520..008F057A:90B35; 008F0580..008F05DD:93B36.
// ECX is the actual live complete raw56 record; EDX is unused padding so the
// three physical arguments remain stack DWORDs. EAX returns that record;
// both branches RET0C. ESI/EDI are saved/restored. No noexcept/unwind promise.
// 0460 stack +4/+8/+C = INPUT/BYTES/fullFLAGS; the other three use
// COUNT/INPUT/fullFLAGS. Only the low flag BYTE selects copy; all high flag
// bits are ignored. Preserve the full uint32_t formal, never narrow to bool.
//
// Require actual whole accepted type8/9/10/11 raw-storage provenance, preserving
// the complete producer's real untouched bytes. This is not a class/vtable ABI.
// Old +20 must be null (e.g. after accepted array-block release on this same
// record) or a uniquely owned live same-current-heap allocation, freed once.
// A retained constructor input and marker +2C=1 do not establish ownership.
// A copied input must be stable/readable for the exact positive byte size,
// disjoint from record, active frame, old allocation freed first and new child.
// With low flag byte zero, INPUT must instead be a genuine separately allocated
// uniquely owned same-current-heap payload of that size. Its caller transfers
// sole retirement responsibility to this record at the native pointer store.
// No caller-managed borrowed lifetime is admitted to later retirement. Retire
// each new payload once through the accepted current release before discarding
// the raw record or handing it to an accepted lifetime path. No alias to old
// allocation is valid, even when the incoming pointer equals the old pointer.
//
// BYTES is positive uint32_t; COUNT is 1..3FFFFFFFh for four-byte groups or
// 1..15555555h for twelve-byte groups. Require nonwrapping valid ranges and
// resources for the exact size. Native DWORD arithmetic has no inserted guard.
// Zero size in either branch is excluded: no null/zero allocation or memcpy
// behavior is inferred and no fallback is added. All payload groups are opaque.
// Same real bag/process/pool/generation and accepted graph/context/lifetime
// contracts remain required if composed. No startup/context substitute is made.
// Valid flat ES, DF clear, sufficient stack, successful returning current
// providers, no faults/throws/reentry/concurrency/trim/teardown/overlap/unwind.
//
// Free old+20 if nonnull, then clear it only after free returns. 0460 loads
// BYTES after free; the other entries compute the full size before free.
// All compare the low flag byte BEFORE size+24 store; 0460 loads BYTES between
// CMP and store. Copy allocates exact size, loads INPUT after allocation and
// stores the actual allocated pointer+20 BEFORE genuine memcpy. No-copy stores
// the exact full supplied pointer. Only eight record bytes [20,28) are written;
// the other 48 retain their prior values. No phase/tag/default/owner/marker,
// lookup/root/context/startup/RAII/rollback action is performed.
//
// No-copy EDX=INPUT; ECX remains root only when old was null, otherwise it is
// the actual free provider's residual. Its final CMP flags mask8D5=44, AF=0.
// Copy ECX/EDX are provider residuals; final ADD ESP,10h defines arithmetic
// flags from entry T: (T-24)+16=T-8. EBX/EBP follow genuine provider normal ABI.
// No blanket DF/ES/FPU/MXCSR preservation, Original private CRT/EH, full binary
// ABI compatibility, parser/selected graph/startup/runtime/game admission.
void* __fastcall reassign_native_scene_property_raw_bytes_008f0460(
    void* actual_record_ecx, void* unused_edx, const void* actual_input,
    std::uint32_t byte_count, std::uint32_t copy_flag_bits);

void* __fastcall reassign_native_scene_property_four_byte_groups_008f04c0(
    void* actual_record_ecx, void* unused_edx, std::uint32_t element_count,
    const void* actual_input, std::uint32_t copy_flag_bits);

void* __fastcall reassign_native_scene_property_four_byte_groups_008f0520(
    void* actual_record_ecx, void* unused_edx, std::uint32_t element_count,
    const void* actual_input, std::uint32_t copy_flag_bits);

void* __fastcall reassign_native_scene_property_twelve_byte_groups_008f0580(
    void* actual_record_ecx, void* unused_edx, std::uint32_t element_count,
    const void* actual_input, std::uint32_t copy_flag_bits);

} // namespace bsp
