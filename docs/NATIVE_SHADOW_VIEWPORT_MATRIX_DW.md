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

Source implementation evidence is retained in
`reports/native_shadow_viewport_matrix_dw.json` and ignored local artifacts.

Primary validation at exact clean source `37b47a7eae7bafd0d4487655d870ac6c022f24c1` passed MSVC Win32 Release and both existing CTests with 2625 unchanged tracked inputs. All351 generated matrix bytes agree with the original after zeroing only the four CALL and seven constant-address operands; every rebound constant bit and both existing four-byte viewport getter bodies also agree. Both new target getter bodies match all four native bytes exactly.

One ignored local differential case executed source and the relocated original351-byte matrix body. The latter called copied unmodified viewport getter bytes and used copied original constants. Unsigned owner dimensions80000003/FFFFFFFB, viewport origin-17/23 and dimensions1301/777 matched all16 output words, surrounding guards, unchanged owner/viewport input storage, output identity, x87 status0020/control037F/empty-stack tagFFFF and unchanged MXCSR1F80. The two new target getters were not executed. No repository tests were added.
