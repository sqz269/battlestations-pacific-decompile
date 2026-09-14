# Native shadow viewport matrix

The complete `A8AAA0..A8ABFE` body (351 bytes) builds a matrix from actual shadow
owner dimensions at +388h/+38Ch and actual viewport origin/dimensions returned
by the existing raw `B1F730` and `B1F740` providers. The two complete four-byte
entries `A8FDA0` and `A8FDC0` read current receiver pointers at +14h and +1Ch.
Descriptive names are hypotheses; these sources do not construct their owners.

The matrix entry receives its actual owner in ECX, output and viewport pointers
in the two public stack slots, returns the output pointer in EAX and uses RET8.
The source adds an unused fastcall EDX parameter. It preserves the native use
of the public viewport argument slot as temporary storage after capturing that
pointer, and preserves the repeated current owner field reads.

The matrix body retains the original instruction sequence, including its x87
stack order, signed viewport FILD inputs, unsigned owner-dimension correction,
float32/float64 spills, SSE moves, sixteen output-store timings and final stack
cleanup. At A8AB21 and A8ABCF the bytes DC C9 mean `FMUL ST(1), ST(0)`; Ghidra's
abbreviated `FMUL ST1` listing does not identify the destination adequately.

Only four calls and seven constant-address operands need source rebinding.
The existing raw viewport getters are reused. The constants are exact native
bits: CE3978 float32 `4F800000` (2^32), D7A280 float64 `3FE0000000000000` (0.5),
and D7A24C float32 `3F800000` (1). All three lie in the original PE's readable,
nonwritable `.rdata` section (characteristics `40000040`). Their source const
objects retain the original memory operand widths; no mutable global binding
or replacement numerical formula is introduced.

Valid actual storage and caller-provided floating-point state remain required.
There is no dimension clamp, pointer check, Boolean conversion, retain or
allocation. Output aliasing and hardware-fault behavior follow the instruction
schedule but have not been separately exercised. No original owner constructor,
whole world-builder, unmasked FPU exception or gameplay proof is claimed.

Validation is pending at this source commit. Evidence is retained in
`reports/native_shadow_viewport_matrix_dw.json` and ignored local artifacts.
