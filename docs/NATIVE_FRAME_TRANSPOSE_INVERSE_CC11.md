# Native frame transpose and inverse translation

`0085DEA0..0085DF5B` is a complete187-byte,47-instruction body. Original ABI:
ECX destination, EDX source, plain RET; no useful semantic return is recovered.
Current callers are006BEE40,007D9360 and007D9C10; those whole callers and their
ownership/lifetimes are not closed by this helper.

The Source entry calls the existing complete raw004134F0 sequential x87 copy,
captures the copied translation, clears it with SSE positive zero, and swaps
three basis pairs using the original mixed FLD/FSTP and MOVSS schedule. It then
calls the existing genuine0042D0D0 normalize=false raw kernel and consumes its
returned vector. Three SUBSS operations subtract from borrowed actual00D7A208
negative zero. There is no replacement matrix, constant, math provider, profile,
normalizer or callback. This describes a transposed-frame translation operation;
general inverse-matrix behavior is not asserted.

The new Source fastcall ABI adds the actual negative-zero cell pointer and RET4.
The original selected-false stack word is retained to preserve local offsets;
the existing raw direction kernel pops only matrix with RET4, so the caller
removes the false word afterward. This extra integer ADD and the borrowed-cell
address load qualify register/EFLAGS/native ABI equivalence. Original and Source
floating instructions and intermediate spill/order remain the same. No semantic
EAX return, native callable profile or binary replacement is provided.

Validation used a fresh manifested fixture containing the whole original
187-byte caller,103-byte copy and187-byte direction body, changing only the two
caller CALL displacements and actual negative-zero operand. The original
normalize=true branch remains present but is not entered. Ambient precision,
rounding, masked x87 stack and MXCSR behavior, raw matrix words and overlap are
the focused risks. Unmasked exceptions, DAZ/FTZ, caller/game binding remain
unverified. No tracked tests are added.

The full Win32 build and all three existing CTests passed at `c96ecf1e0f0d7f5193a220cd20459e38924ab1d0`. Four fresh actual translation units,41current Source/header/fixture inputs and three current libraries plus the original PE were stable before/after linking. The PE32 asInvoker probe passed46full-original composition cases (619checks including PE bounds/setup):24ambient PC/RC/occupancy cases,8independent MXCSR-rounding cases,8same/partial-overlap/unaligned cases, and6signed-zero/NaN/denormal cases. All256backing bytes, CW/SW/FTW/all8raw80-bit x87 fields, MXCSR and all128bits of XMM0..4 matched. Root independently checked all477disk/live bytes; the Source COFF floating schedule matches with only the actual-cell addressing qualification, two real helper relocations, and an exact103-byte raw-copy provider. Detailed hashes and Source ABI differences are in the report.
