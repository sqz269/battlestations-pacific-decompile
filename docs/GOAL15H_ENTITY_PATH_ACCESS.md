# Complete raw entity/path access entries

Addresses: `007AC9D0`, `00923810` (owned). Names are hypotheses.

These entries operate on actual raw entity/parent storage. Path selection makes
the four real indirect calls through the receiver's current vtable+5Ch; it
does not consume previously computed type flags or a host callback façade.
The ancestor entry returns the pointer in EAX, preserving its signed counter
tests and DWORD decrement wrap. Existing semantic interfaces and historical
records remain distinct and unchanged.

| Entry | Inclusive body | Coverage | Original physical ABI |
| --- | --- | --- | --- |
| `007AC9D0` | `007AC9D0..007ACA2C`, 93B/43 instructions | complete | ECX entity; no stacked input; EAX pointer; plain RET |
| `00923810` | `00923810..00923830`, 33B/13 instructions | complete | ECX entity; one stacked signed32 level; EAX pointer; RET4 |

Source declarations in `include/bsp/native_entity_path_access.hpp` are
`get_native_entity_path_interface_007ac9d0(entity, unused_edx)` and
`get_native_entity_ancestor_00923810(entity, unused_edx, levels)`, both returning
`void*` through Win32 fastcall. The explicit unused EDX adapters are not
Original arguments. `src/native_entity_path_access.cpp` contains both complete
literal kernels. No raw owner/vtable schema is invented; the consumed storage
is a vtable word at +0 and a parent word at +3Ch.

## Path predicate schedule

Null entity returns zero without touching entity storage. For nonnull input,
ESI captures the original receiver. Before EACH reached kind47h/48h/49h/4Ah
predicate, the body reloads `[ESI]`, then `[current_table+5Ch]` into EDX, pushes
the kind DWORD and calls EDX. The first call retains incoming ECX; later calls
explicitly restore ECX from ESI, so earlier predicates may clobber ECX and
change the published vtable. The receiver capture itself is not replaced.
The raw method contract requires the actual target to be callable with ECX
receiver/one stacked kind DWORD/callee RET4 and ordinary callee-saved register
preservation. EDX contains the actual method target, not an injected context.
Only AL is tested; nonzero high EAX bytes with AL0 remain false.

| Kind | Physical indirect instruction | True result |
| --- | --- | --- |
| 47h | `007AC9DE FF D2` | captured entity+1E4h |
| 48h | `007AC9F5 FF D2` | captured entity+170h |
| 49h | `007ACA0C FF D2` | captured entity+310h |
| 4Ah | `007ACA23 FF D2` | captured entity+1E4h |

The first true predicate short-circuits. If all four AL values are zero the
answer is null. LEA computes the pointer by native modulo32 addition; there
is no C++ pointer subtraction/offset projection, extra bounds check, cached
vtable, or precomputed predicate result. Every reached receiver/table cell and
call target must be valid under this raw contract. No actual game entity
factory, vtable producer, hierarchy membership or class name is established.

## Parent chain and return correction

The ancestor body first reads `[ECX+3Ch]` into EAX unconditionally, reads the
stacked level into ECX and decrements it by one modulo32. Null parent returns
immediately; otherwise a signed `TEST ECX; JLE` controls each later parent
load/decrement. Thus ordinary zero/negative levels return the first parent,
levels1 returns first parent, and positive levels may follow more parents.
INT_MIN decrements to 7FFFFFFF and therefore follows the positive-counter path;
it does not use C++ signed-overflow arithmetic or clamp to one step. Every
reached parent cell must be backed; cycles consume the finite counter without
a cycle detector. A null receiver has no special case. Native `8B FF`
(`MOV EDI,EDI`) at the alignment position and the final RET4 are retained.

The old pseudocode's `void` return misses the live EAX pointer. The literal
body and caller EAX tests/stores/dereferences establish the physical result.
Historical `entity_ancestor_00923810(Raw,int)` and path-offset/host APIs remain
unchanged; their existence is not substituted for these raw entries.

## Complete caller evidence

The live census contains 19 direct path calls and 69 direct ancestor calls.
All 88 sites have saved instruction contexts and independently queried live
containing bodies in the report. All ancestor callers currently pass level1:
68 push literal1, and the other loads EDI=1 before pushing EDI. That observed
usage does not narrow the complete entry's signed32 argument domain.
The relevant avoidance associations include these listing rows:

```text
0041D201 CALL 007AC9D0
0041D06D CALL 00923810
0041D07B CALL 00923810
0041D094 CALL 00923810
0041D3A2 CALL 00923810
```

The report attributes them to their actual containing functions, alongside
all other live callers. Their functions are consumed evidence, not additional
reconstructions in this packet. Two historical evidence details are corrected
by append-only evidence: the earlier path comment read one caller while the
complete census has 19, and old `entity_command_arms.hpp` cites 007ACA21 for
4Ah whereas that is MOV ECX,ESI; the actual indirect instruction is 007ACA23.

## Verification and boundaries

Strict MSVC x86 `/std:c++17 /EHsc /MD /W4 /WX /permissive- /fp:strict /O2`
compile PASS. Live listings have zero gaps. Ignored
`local/goal15h_entity_path/verify_bytes.py [optional_registered_object]`
compares all 93+33 COFF bytes with independently captured live Ghidra bytes
and the corresponding installed PE bytes. All126 bytes PASS with no masks,
normalization or relocations, including all indirect calls, AL tests, receiver
and table reloads, modular offsets, signed loop, identity instruction and RETs.
This is exact static body evidence, not Native runtime/virtual deployment proof.

One ignored combined fixture `local/goal15h_entity_path/entity_path_probe.cpp`
strict-compiles and PASSes. Its explicitly local Source vtables contain real
Source predicates under the raw register/stack contract. Each predicate checks
ECX/EDX/kind, republishes a different table and deliberately clobbers ECX. The
fixture tests all four true branches/all false/null, high EAX bits with AL0,
noncanonical nonzero AL, exact offsets and call order, plus valid parent chains,
zero/negative/positive levels, INT_MIN wrap and a null parent. The fixture's
callable table is local test storage, not a production callback substitute or
evidence of an actual Native entity provider. It does not test an unbacked
receiver, near-address-space wrap, Native exceptions/SEH or gameplay.

The fixture command is `local/goal15h_entity_path/run_probe.cmd`, using /MD and
`/link /MANIFEST:EMBED`; an optional first argument supplies the registered
archive instead of the worker object. The report's 88 direct-call rows and
the five markdown association claims PASS with zero failures/unchecked claims.
Primary owns registration of
`src/native_entity_path_access.cpp`, full registered Win32 build/existing tests,
registered-object comparison/archive fixture, Original prototype annotations,
save/export refresh and main integration. The worker changes no Ghidra state,
CMake or shared packet registry. No Native virtual drop-in, factory, exception,
whole-program or gameplay equivalence is claimed.
