# Native DWORD-vector allocation and length error

Addresses: `00553260`, `00556C20`, `00C6D790`.

The three complete Source entries use the accepted current allocation and owning
legacy-exception services. Existing STL native names and earlier qualified
storage fragments are preserved. This packet does not reconstruct the parent
insertion, construction, copy construction or Append routines.

| Native entry | Coverage | Original ABI | Source interface |
| --- | --- | --- | --- |
| `00553260` | complete at the accepted current-CRT boundary | ECX unsigned count; incoming EDX and stack unconsumed; EAX raw allocation; plain RET | `allocate_native_dword_vector_00553260(count, unused_edx)` |
| `00556C20` | complete at the accepted owning Current C++ exception boundary | no consumed public input; no normal return | `throw_native_dword_vector_length_error_00556c20()` |
| `00C6D790` | complete cleanup operation through explicit existing storage | LEA ECX,[EBP-50h]; tail JMP; original EBP/private action frame | `cleanup_native_dword_vector_length_message_00c6d790(completed_message)` |

All interfaces are new Source APIs. The explicit cleanup reference replaces a
private frame lookup; it does not expose the original unwind-entry ABI.

## Whole-body equivalence and allocation

All 25 allocator instructions match accepted `00BCFEB0` after only local branch
relocation and relative transfer encoding are normalized. Registers, memory accesses,
immediates and resolved call targets agree. Passing the same unsigned count and
returning raw `void*` is justified by this complete body comparison. No pointer
count conversion, container overlay, typed pointer array or header interpretation
is introduced. The reference allocator's historical name does not establish the
contract.

Count zero still invokes allocation with zero bytes. Positive count is admitted
when `FFFFFFFFh / count >= 4`; counts through `3FFFFFFFh` request exactly
`count*4` native and host bytes. Above that limit, the original constructs a
12-byte bad_alloc payload using a null-message pointer slot, writes vtable address
`D6923C`, and throws with ThrowInfo `E03CC0`. The accepted Source service instead
uses its existing `std::bad_alloc` policy. Its allocation metadata does not change
the byte count: `singleton_lifetime_allocate` consumes the requested host bytes
in the existing malloc/_callnewh/retry/free domain.

The separate 8-byte string allocator `0042BF40` differs in exactly two native
instructions: the allocation LEA scales by 8 and the division quotient compares
against 8. Its `1FFFFFFFh` limit cannot be reused by scaling or reinterpreting this
packet's count.

## Length-error ownership and unwind action

All 23 length-helper instructions match accepted `00BD0590` and `00431F70` after
only EH-handler address relocation and relative encodings are normalized. The
helper uses an actual uninitialized 1Ch legacy SBO temporary, writes capacity15,
length0 and inline NUL, then assigns 18 message bytes from `CE37E0`
(`vector<T> too long`; the stored source has a separate trailing NUL). State0 is
armed only after counted assignment returns.
Actual `00411700` constructs the owning 28h payload, the native vtable address is
`D69260`, and ThrowInfo is `D83F98`.

Handler `C6D798` selects FuncInfo `D97B58`: magic `19930522`, max state1, no try
blocks, flags1. Map `D97B50` sends state0 to -1 through `C6D790`. That entire
8-byte action is `LEA ECX,[EBP-50h]; JMP004072D0`. The accepted full length helper
already destroys its completed temporary through the corresponding guard. The
new explicit action calls actual SBO destruction once on supplied existing
storage; it is never added as a second guard around the full helper.

The Source exception remains the existing `NativeSingletonVectorLengthError`,
which owns the actual 28h legacy payload and uses the accepted actual copy and
destruction services. It has a new C++ catch type and RTTI. It is not a
`std::length_error`; the stored native vtable word is address data, not a host
vtable. The action inherits actual Source destruction's noexcept contract.

## Current native transfers and producers

The JSON report records every owned transfer, all compared-reference transfers,
and all six current external producer calls with their containing function.

| Containing entry | Current native instruction | Role |
| --- | --- | --- |
| `00553260` | `00553271 CALL 00BF681B` | original operator_new with byte count |
| `00553260` | `00553299 CALL 00BF6340` | original exception null-message constructor |
| `00553260` | `005532B0 CALL 00BF6885` | original bad_alloc throw |
| `00556C20` | `00556C58 CALL 00408720` | actual counted SBO assignment |
| `00556C20` | `00556C6D CALL 00411700` | actual logic-error payload construction |
| `00556C20` | `00556C84 CALL 00BF6885` | original length-error throw |
| `00C6D790` | `00C6D793 JMP 004072D0` | tail transfer, completed-message destruction |
| `00557200` | `00557226 CALL 00556C20` | nonzero count exceeds 3FFFFFFFh |
| `00557200` | `0055722F CALL 00553260` | count unchanged in ECX; EDX zero |
| `00557590` | `005575E3 CALL 00556C20` | captured SAR2 source count exceeds limit |
| `00557590` | `005575EC CALL 00553260` | same captured count; EDX zero |
| `00697990` | `006979F4 CALL 00556C20` | requested insertion count exceeds remaining length |
| `00697990` | `00697A55 CALL 00553260` | computed growth count unchanged; EDX zero |

The producers establish unsigned DWORD element counts and raw byte allocation;
their old names do not establish a different element width. Producer snippets
are separate evidence, not whole-parent Source coverage. Native allocation has
no stacked count input or RET-immediate; its internal operator_new push is
cleaned by `ADD ESP,4`. The Source EDX reservation does not consume its value.

## Verification and remaining limits

Fresh read-only queries verified project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. All 198 owned code bytes/50 instruction starts match
disk and live memory. The full compared-code proof is 624 bytes/156 starts;
351 separate producer bytes add 134 starts. Together the current proof covers
975 code bytes/290 starts, 24 CALL sites and six EH tail JMPs. The report freezes
the raw bytes, hashes, body ranges, EH maps, constants and Source hashes. The
remaining 467 checked metadata bytes are counted separately from code.

The MSVC Win32 build passed, and the three existing suites passed in 7.36 seconds;
the JSON report records the checked Source hashes. No new tests or native/game
execution are introduced. These checks do not prove the original static-CRT
heap/new-handler state, native bad_alloc/length_error RTTI or throw identity,
BF6885/FH3/SEH transport, private-frame aliases, fault/unwind-failure behavior,
drop-in binary ABI compatibility or gameplay behavior. Original BF6885 and
BF6B43 runtime bodies remain outside the accepted Current C++ service boundary.
