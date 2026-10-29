# Dyn endpoint initialization and vector storage cleanup

The complete `00C4CE60..00C4CF1A` constructor (187 bytes, 66 instructions)
and its real unwind dependency `0040A050..0040A05D` (14 bytes, seven instructions)
now have explicit C++ interfaces. Names and endpoint interpretation remain
hypotheses. Native constructor inputs are ECX token, DL flag, stack endpoint
and six-word bounds, RET8; EAX returns the endpoint. The helper takes its
actual vector data-word address in EAX and uses plain RET. Neither new public
interface supplies those original register ABIs or a native class/vtable.

All original instructions, helper bytes, the handler and cleanup action were
read through the verifying existing bsp project/program client. The full
constructor and helper bytes also match the installed original PE. Addresses,
hashes and call sites are in the report. No vendor body or unwind funclet earns
new reconstruction credit.

The constructor writes token and low flag byte before six interleaved raw-word
bounds loads/stores. These remain volatile individual operations, retaining
alias effects and raw float bits. It zeros vector data/count/capacity at
30/34/38, then publishes capacity10 before genuine BF55BE allocation of40 bytes.
The native constant comparison0<10 makes its apparent skip-growth edge dead
on ordinary entry. Allocator-visible count and data are still reloaded during
copy; a zero computed destination skips only that write. Old data is captured
again, conditionally freed through genuine BF6989, then the new pointer is
published. Final byte2C is cleared only after normal completion. No allocation
success check, fallback, field repair or synchronization is introduced.

Native FuncInfoE040A0 has one state, with E04098 mapping state0 to previous-1
and actionCC823A. That action adds30h to the actual endpoint and tail-jumps to
0040A050. The established helper captures current vector data once, frees it
iff nonzero and leaves every vector field unchanged. An ordinary C++ catch
composes this actual helper and rethrows. It neither frees a retained new
buffer nor rolls back native partial mutation. Original FH3/SEH frames, access
fault handling, cleanup-that-itself-throws transport and exact exception/fault
addresses remain unproved. In particular native state0 also covers the final
flag memory store; /EHsc C++ handling does not claim native SEH equivalence.

An independent worker reviewed all66+7 instructions, actual source/header and
live EH data while composing the real dependency in C55A60. It found no blocking
mismatch for the stated normal/C++ cleanup contract. Its evidence is retained
with packet28d67fc12. The common Win32 Release build and both existing suites
pass (12.61 seconds); no new test cases were added. A manifested link-only
executable forces both actual entries and the separately recovered pointer
retirement entry, resolving the real C4DC60 destructor from bsp_core. It was
never executed. All three new entries are absent from the actual game map.

Status is compiled/link-tested and unwired. Caller object/bounds/CRT custody,
native ABI/exception transport, allocation-failure execution, native differential
and gameplay remain open. Only the constructor reduces the original29-body
missing census; the helper is one separately recovered in-house body.
