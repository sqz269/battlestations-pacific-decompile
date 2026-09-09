# Render entry comparison

`00b51df0` selects `00b51b00` for batch index zero and `00b51ab0` for every
other index when queue sorting is enabled. Neither index has been established
as an opaque or transparent pass.

`00b51b00` compares the unsigned 64-bit field at entry+20h in ascending order.
Assembly first compares the high DWORD (+24h), using JA to reject and JC to
accept; equal high words fall through to the low DWORD comparison, where JNC
rejects greater/equal. Equal keys return false. This is not signed ordering.

`00b51ab0` reads a signed DWORD through entry+4 -> object+20h -> object+7Ch ->
field+B0h. Unequal values use SETL, establishing signed ascending order.
Equal values compare the floats at entry+14h in descending order. FCOMIP
compares left to right and JBE rejects less, equal and unordered; consequently
NaN comparisons return false, as do equal values and signed zero pairs.
The material field and secondary float have no established domain names.

Both functions take entry pointers directly in ECX and EDX and return with
plain RET. Their predicate is in AL. On the unequal-material path, 00b51ab0
only replaces AL; upper EAX retains bits of the right material field. Treating
the entire EAX value as a 32-bit truth value would be incorrect.

`include/bsp/render_sort.hpp` and `src/render_sort.cpp` reconstruct only these
predicates using projected, typed fields. They are new C++ interfaces, not
binary-compatible overlays or hooks. Native pointer traversal, key generation,
queue dispatch and the native sorting algorithm remain outside this port.
The boolean comparison behavior is reconstructed; x87 status and exceptions
are not reproduced. In particular, NaNs within a material group prevent strict
weak ordering, so this predicate must not be handed unrestricted NaN inputs
to `std::sort`. No replacement sorting algorithm or tie permutation is claimed.

The caller's sort helper at `00b1dce0` uses a 32-element small-range cutoff,
a shrinking partition budget and separate fallback helpers. Its decompiled
register arguments are incomplete; it has not been reconstructed here.

The evidence report records saved-image/original-file byte comparisons for
both comparator bodies and the selector body. Missing functions were created
only at the two comparator addresses; their prior function state was absent,
and existing labels were LAB_00b51ab0 / LAB_00b51b00. Proposed descriptive
names remain evidence-based hypotheses, not recovered symbols. This port is
not native differential-tested or game-validated.

Integration validation: MSVC Win32 Release build passed with /W4 /WX /fp:strict.
Both existing CTest checks passed; they cover math, not these predicates.
No new tests were added. Comparator names and comments were saved to Ghidra.
