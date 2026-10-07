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

Validation is pending a fresh manifested fixture containing the whole original
187-byte caller,103-byte copy and187-byte direction body, changing only the two
caller CALL displacements and actual negative-zero operand. The original
normalize=true branch remains present but is not entered. Ambient precision,
rounding, masked x87 stack and MXCSR behavior, raw matrix words and overlap are
the focused risks. Unmasked exceptions, DAZ/FTZ, caller/game binding remain
unverified. No tracked tests are added.
