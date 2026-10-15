# Actual scalar-tree range erasure

The complete ordinary `005572F0` entry now delegates to the accepted complete
`erase_native_int_pointer_tree18_range_0086ee50` service. This is distinct from
the earlier consumed full-path fragment in `native_input_nested_int_subtree.cpp`.
That fragment and its whole `55AF60` caller remain unchanged. The correct
`STL_IntScalarTree_EraseRange_N18_005572F0` identity is retained; the new C++ name
is a hypothesis. The generic AV projection remains separate.
The earlier fragment record retains its historical status; complete ordinary
coverage is recorded separately.

| Entry | Coverage | Binding |
| --- | --- | --- |
| 005572F0 | Complete ordinary function, both full and partial paths | Complete accepted 0086EE50 |

The native entry has 200 body bytes and 76 instructions in `005572F0..0055736E`
and `00557370..005573B8`; the intervening `0055736F` NOP is outside the typed
body. ECX is the actual tree, EDX is unconsumed, and the five physical stack
words are output, first owner/node and last owner/node. Both normal returns
use `RET14h` with EAX equal to output. The actual tree stores opaque/head/count
at 0/4/8. Its 18h nodes use links at 0/4/8, opaque key/value at C/10, color14 and
nil15. No payload release is added.

Fresh approved Ghidra queries verified project `C:/Users/sqz269/bsp.gpr` and
program `/battlestationspacific.exe`. Eight complete bodies agree with the
installed PE and every current Capstone instruction start. The complete
201-byte range envelopes match after six exact E8 displacement fields. The
complete subtree, increment and iterator-erase pairs match with separately
proved target and EH-handler substitutions. Every other byte, including local
branches and the NOP, agrees. The report retains the full masks and provenance.

| Native site and target | Accepted corresponding effect |
| --- | --- |
| `0055730A CALL 00BF6713` | Returning current CRT invalid-parameter diagnostic |
| `00557326 CALL 00BF6713` | Returning current CRT invalid-parameter diagnostic |
| `0055733A CALL 00554F10` | Complete scalar subtree, matching actual 0086AA60 |
| `0055737A CALL 00BF6713` | Returning current CRT invalid-parameter diagnostic |
| `00557389 CALL 005531F0` | Complete checked increment, matching actual 00869A20 |
| `00557397 CALL 00556DC0` | Complete iterator erase, matching actual 0086E8A0 |

First owner is captured, then the current head and head.left are captured before
the first diagnostic. First node is read afterward and compared with that
captured left bound. On a match, last owner is read/tested and the current head
is captured before its diagnostic. Last node is read afterward and compared
with that captured head. Diagnostics may return; the service preserves these
captures and raw reloads instead of validating through new guards.

The full branch destroys the current root, reloads head and sets parent=self.
It reloads head before zeroing count and setting left=self, then reloads for
right=self. It captures head.left before storing output owner then node. The
valid empty full range still performs the nil-root cleanup and reset.

The partial branch validates owner before comparing cursors on every iteration.
It advances the actual first owner/node argument slots, then erases an OLD
captured iterator passed by value with a private eight-byte output. The child
advances its own copy. The parent ignores that result and EAX, reloads its own
first node then owner, and repeats. This is not a `first = erase(first)` loop.
At completion it stores captured owner then captured node to output. Raw output
aliases supported by the accepted service retain this exact write order.

The parent has no local EH state, catch or rollback. An escaping child exception
leaves earlier completed erasures committed. The iterator child constructs the
actual owning 28h error payload with the 27 counted message bytes, actual SBO,
vtable and throw profile. Its state-zero cleanup uses 4072D0; both complete FH3
maps/actions and the message/throw spans were checked against current live and
disk bytes. The accepted owning C++ transport is retained.

The Source uses current CRT allocation/free and diagnostics, current C++ RTTI
and cleanup, and its own physical argument/private frame. It does not establish
original FH3/SEH, faults, spill-address aliases, volatile-register residue,
native exception/heap identity, game execution or drop-in ABI compatibility.
The original listing already contains both complete branches; the known child
decompiler ABI/memory-effect limitation does not require body repair.

The exact Source passed `./scripts/build.ps1` and all three existing suites
(`reconstructed_math`, `native_math_differential`, `tool_tests`) in 8.04s.
The compiled wrapper is a 9-byte tail forwarder to the accepted whole
service, with any EBP frame restored before transfer, unchanged ECX/EDX and all
five stack words. The COFF relocation identifies the exact accepted target;
its zero displacement is not interpreted as a runtime destination. This checks
Source frame behavior and does not prove the original game ABI.

All six native transfer rows and six Markdown claims pass fresh verification;
diff checks pass. No new tests were added. The existing suites are build and
regression coverage and do not execute this new entry.
