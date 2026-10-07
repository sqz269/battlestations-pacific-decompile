#pragma once

namespace bsp {

// Complete 007ED260 raw entry: ECX is the actual squadron, EDX is unused,
// no stack arguments, RET. This preserves the physical native stack scan.
// Ordinary storage: live signed count at +3CC, live member pointer cells at
// +3D0, and occupied planes with writable index +9D0 and ordinal +9D8.
// Stable counts 0..5 and old indices 0..4 are the tested storage domain.
// The five-member scan can read beyond its five local slot words into the
// actual return address and following caller words. The caller must retain
// readable stack storage through the first nonnegative candidate it reaches.
// No capacity policy, copied slot array or substitute caller words are added.
// Native class admission, faults, concurrent mutations and game binding are
// unvalidated. The return value and volatile registers are native scratch.
void __fastcall native_plane_squadron_reindex_007ed260(
    void* actual_squadron, void* unused_edx);

} // namespace bsp
