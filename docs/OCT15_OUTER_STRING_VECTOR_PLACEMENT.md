# Native outer checked string-vector placement wrappers

Packet `oct15_outer_string_vector_placement` covers `006A6810`, `006A6B60`,
and their separately leased unwind actions `00C7F4D0` / `00C7F510`. Both
complete ordinary effects directly use accepted actual
`construct_native_outer_string_vector_copy_0055b150`. There is no host vector,
placement-copy callback, borrowed assignment, new allocator or extra cleanup.
Native conditional EAX residue is documented rather than converted into a
fabricated null pointer result.

| Entry | Inclusive native body | Original physical inputs / cleanup |
| --- | --- | --- |
| `006A6810` | `006A6810..006A684F`,64 bytes/17 instructions | ECX slot,EDX source; no stacked input; plain RET |
| `006A6B60` | `006A6B60..006A6BA7`,72 bytes/19 instructions | entryECX/EDX unconsumed; stack(slot,source); RET8 |
| `00C7F4D0` | `00C7F4D0..00C7F4E0`,17 bytes/7 instructions | first wrapper's native parent-frame unwind action |
| `00C7F510` | `00C7F510..00C7F520`,17 bytes/7 instructions | second wrapper's native parent-frame unwind action |

Complete pseudocode/listings, handler/maps/action bytes and the real child
Source/header were inspected. Live queries verified project `bsp`, configured
at `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, language and
image base before analysis/export. The two normal listings have no gaps and
reach explicit returns; each action's complete listing is also present. No
flow/no-return repair was needed. Ghidra and the installed game were not changed.
Names `STL_ConstructStringVectorVector_006a6810`,
`STL_ConstructStringVectorVector_006a6b60`, `Unwind@00c7f4d0`, and
`Unwind@00c7f510` are retained. Descriptive names are not recovered symbols.

Both wrappers test destination slot for zero. A null slot performs no child,
source-header, destination-header or pool operation; there is no Source
evaluation of pointed storage or fabricated owner. Nonnull first-wrapper
instruction `006A683C CALL 0055B150` passes slot in ECX and source on stack,
with EDX still holding source. The new Source forwards those source bits to
the constructor's explicitly unused EDX word. The stack-wrapper instruction
is `006A6B92 CALL 0055B150`: ECX is the stacked slot, stacked source is loaded
only on the nonnull path, and incoming EDX remains unchanged/unconsumed.
Source reserves the original unused register positions and forwards that word.

The actual destination is a16-byte outer checked-vector header with
opaque/proxy+0,begin+4,end+8,capacity+C. Its backing contains actual16-byte inner
checked-vector headers, each containing actual8-byte pooled strings. Accepted
`0055B150` establishes the relevant fields through actual child services,
preserves +0 preimage, retains count-before-clear/current reads and no-self-guard
effects, and supplies its own ordered ownership/exception cleanup. These
wrappers add no whole-header initialization, copying, old-buffer destruction,
alias repair or retry. Nonnull Source requires admitted construction/source
storage, actual raw-pool publications/current-CRT ownership and all accepted
child diagnostic/zero-copy/exception limits. Overlap remains governed by that
actual constructor, including native ownership loss where the child exposes it.
The child was accepted as `a276944a8` with its real cleanup dependencies before
the placement build; no temporary copied header or unresolved binding is used.

Both native prologues load EAX from entry FS:[0] before installing their own
registration. Null branches skip all later EAX writes and leave that previous
exception-chain value in EAX. It need not be zero. Nonnull leaves the actual
constructor's EAX=slot. Neither current live xref set contains callers, so no
semantic result type is established. Public Source entries are void, with no
promise to reproduce these private register bits and no synthetic
incoming-EAX/exception-list parameter. The original C++ return type remains
unrecovered. These are distinct facts from normal side-effect completeness.

The old second-wrapper name evidence says cdecl. Its final bytes are
`C2 08 00`: native callee cleanup of two physical stack words, consistent with
a stdcall entry, not cdecl. Its prologue duplicates slot into a local and the
original first argument word; private-frame alias effects are not modeled by
the new Source interface. New Source fastcall reserves unused ECX/EDX then
keeps original stack(slot,source), appending context, so Source RET0Ch replaces
native RET8. First Source fastcall adds a context stack word and returns RET4
instead of native plain RET. Neither is a drop-in original binary ABI.

First handler `00C7F4E1` loads FuncInfo `00DAC924`, then jumps to `00BF6B43`.
Its map `00DAC91C` has state0 to -1 through action `00C7F4D0`. Second handler
`00C7F521` loads `00DAC984`, whose map `00DAC97C` has state0 to -1 through
`00C7F510`. Both complete36-byte FuncInfo tuples have magic `19930522`, one
state, no try/catch/IP/ESType lists and flags1. State0 is armed before each
null check branch outcome, and the two slot-word placements belong to that
construction action. The first action's operands are [EBP-14h]/[EBP-10h]; the
second's are [EBP+4]/[EBP-10h]. Instructions `00C7F4D8 CALL 00401130` and
`00C7F518 CALL 00401130` each pass the two words, remove8 stack bytes and RET.
Actual `00401130` is one C3 RET. It neither frees nor touches a header. Thus
there is no observable placement-delete cleanup to add, and Source introduces
no fake deallocator, standalone callable unwind entry or extra catch. Native
action records are parent-bound evidence for this proven elision.

Raw installed PE extraction asserts all four targets, exact RET/RET8 bytes,
initial MOV EAX,FS:[0], both full FuncInfo/map/action structures and the actual
RET-only target. The JSON report includes full raw spans and extraction
provenance. Source C++ exceptions propagate through the accepted child's own
cleanup; original FH3/SEH/RTTI/catch, private-frame/register/fault and
cleanup-failure unwind identity remain qualified. No native wrapper/action has
x87/FP arithmetic; NaNs are not involved. Older projections/history remain
separate and are not modified.

After the accepted dependency merge, the short registration lease
`oct15_outer_string_vector_placement_cmake_registration` was queued, granted,
used for the startup append, then released to the next queued integrator.
`./scripts/build.ps1` passed the exact Source and all three existing suites:
reconstructed_math, native_math_differential, tool_tests (10.52s total).
Actual Release COFF confirms both null branches, unchanged ignored EDX,
direct actual constructor linkage, and Source RET4/RET0Ch. It has no own
cleanup/EH frame or synthetic placement delete. Its void null branches leave
entry EAX rather than loading native FS:[0], illustrating the explicit
register-ABI qualification. Both JSON and Markdown exact call checks pass4/4;
Markdown has no unchecked claims. No new tests are added; existing suites are
regression/seed coverage, not packet-specific native execution. No original
binary ABI, exception execution, startup or gameplay proof is claimed.
