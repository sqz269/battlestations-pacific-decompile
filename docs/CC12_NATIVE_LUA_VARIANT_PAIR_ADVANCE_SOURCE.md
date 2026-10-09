# CC12 raw Lua variant pair-advance Source candidate

This packet adds `advance_native_lua_variant_pair_006eda20(void*)` as a
candidate MSVC Win32 naked fastcall interface for the complete 99-byte /
38-instruction body. ECX is the actual pair address. There is no EDX dummy,
public stack argument, recovered class, copied backing or semantic result.
The candidate awaits integrator registration, normal build and complete
emitted-object review; no Source admission or Original ABI credit is added.

Baseline: `9b624342e14ef3a89c59e9a204a42491dcb0c29b`. Exactly four new files
are owned: the header, implementation, this document and its
[report](../reports/cc12_native_lua_variant_pair_advance_source.json).
Earlier 691-byte parent and 99-byte readiness snapshots are unchanged.

## Complete body and actual provider

Root Astra's complete-body gate covers `006EDA20..006EDA82`. The captured
99 bytes match the Original PE and decode to 38 instructions:
`2f2b51ef06aedbbaca0ccb3c723439fe65cb551e56a110f29bbcf8efcaec210e`.
Every saved listing start agrees with the raw decode. There is one physical
CALL, one external tail JMP and two local plain RET instructions; the saved
metadata's call count of two is retained as a separate metric.

Both transfers use the actual declared
`invoke_native_invalid_parameter_00bf6713()`. That zero-argument cdecl Source
entry is admitted and registered at this baseline: 17 emitted bytes /
nine instructions, with an actual `_invalid_parameter_noinfo` import. It
requires no fabricated runtime receiver or service object. Its XOR/five
PUSH-zero/CALL/ADD-14h/RET schedule treats the five physical zero words as
unused Source callee padding, not invented semantic API arguments.

The old pair-readiness report's description of only a typed runtime member
is historical and has been superseded by this concrete entry. Its earlier
Native and version-specific results remain unchanged. The new entry still
uses current CRT handler, returning-call and failure policy. It does not
reproduce Native `BF66EF` encoded-global, debugger, dynamic-handler or Watson
behavior. The pair caller cannot strengthen that into a Native runtime or
exception-policy guarantee. Neither interface promises noexcept or noreturn.

## Preserved schedule and Source encoding expectation

All 38 owned instructions map directly. The only redirected operands are
the ordinary CALL at `006EDA28` and the tail JMP at `006EDA37`, both to the
admitted Source provider. The three explicit bytes `8D 49 00` retain the
physical `LEA ECX,[ECX+disp8(0)]` at `006EDA4D`, following the already admitted
small-link helper's encoding method. This prevents assembler shortening and
does not add a semantic operation. Local branches retain their exact targets.

The expected candidate is 99 bytes / 38 decoded instructions with two REL32
redirects. That is a Source encoding expectation, not a compiler result;
the integrator must verify the actual extent, complete bytes, indexed COFF
relocations, symbol ownership and Core membership. No build, probe, forced
retention or production consumer is introduced here.

Let B be the actual pair address. The first full-DWORD guard reads `[B]`.
If zero, the ordinary provider call may return. The next operation then
loads current `[B+4]`; it neither rechecks `[B]` nor substitutes an entry
snapshot. A nonzero byte at the loaded node's `+31h` selects the restored-
frame tail: POP current saved ESI first, then JMP the provider without
resetting ECX or EDX and without a local continuation.

With that byte zero, the body loads the current node's right word at `+8`
and tests the selected child's byte `+31h`. A zero byte selects descent:
load the child's current `+0` word, test its flag, execute the physical LEA
only on the initial zero case, then retain the original loop and final
`[B+4]=ECX` store. The backedge bypasses the identity LEA.

A nonzero right-child flag selects ascent. The body loads the current
node's `+4` parent and tests its byte `+31h`. On each continuing iteration,
it reloads current `[B+4]`, compares it with the parent's current `+8` word,
and conditionally writes that parent to `[B+4]`. It then copies the parent
to EDX and reads `[EDX+4]` after the pair write. No early parent capture is
substituted. Every ascent exit retains the final `[B+4]=EAX` store. The
two backedges and every intermediate write remain explicit.

Every flag byte is tested against zero; any nonzero value takes the
nonzero branch. No color field, tree invariant, unique-successor rule,
fixed iteration count, validation loop or rollback is invented.

## Stack, machine values and raw backing

Let S be entry ESP. PUSH ESI stores at `S-4`; ordinary work uses that ESP.
The optional CALL enters the admitted provider at `S-8`, with no new public
argument. Compatible normal return restores ESP to `S-4`; continuation
requires usable ESI=B. Either local exit pops the actual current word at
`S-4`, then plain RET uses the current entry return backing and leaves
ESP=`S+4` on ordinary return.

The tail first restores ESI and ESP=S. Its JMP enters the provider with the
original return word and no extra return address. If the provider returns
normally, its RET returns directly to the original caller. Its own policy
may also prevent return; the pair adds no alternate continuation.

The Source provider's five padding pushes and CRT call require backing
through outer `S-20h` on the ordinary-call path and `S-18h` on the tail path,
plus the CRT's deeper frames. No additional pair-interface words are added.
Saved-register and control-word placement within this caller matches its
owned schedule; actual Source child code addresses, stack activity and
runtime effects remain separately qualified.

Void supplies no semantic result. Descent physically leaves EAX as the
nonzero-flag stopping child and ECX as the value stored in pair+4. Ascent
physically leaves EAX as the final stored parent candidate; ECX/EDX retain
the exact path-dependent loads in the original schedule. Incoming EDX has
no owned use before definition, but can reach the optional provider. The
body has no owned EBX/EDI/EBP writes. Current saved ESI is popped, rather
than restored from an independent immutable snapshot.

Both local returns retain ZF=0 from their final comparison. Byte exits
retain CMP-byte-with-zero flags; the link-inequality exit retains the
actual DWORD subtraction flags. MOV/LEA/POP/RET do not replace them. Tail
results and flags belong to the admitted provider. No whole-call DF/x87/
SIMD preservation or Original flags equivalence is inferred.

The caller supplies live readable pair+0 and readable/writable pair+4
DWORDs, plus every selected node `+0/+4/+8` DWORD and byte `+31h`. The
highest selected byte is a 50-byte addressed span, not an allocation size
or a recovered node type. Raw x86 accesses add no alignment validation or
stronger C++ struct-alignment promise. Caller backing must support the
actual accesses and subordinate requirements.

Pair+4 writes can alias later node reads, including the parent load after
an intermediate store, or current saved ESI/return backing. Repeated loads
are retained for those effects. Source child effects and aliases also
prevent a transitive guarantee that pair+0 remains unchanged. Invalid
backing, cycles, memory faults and nonlocal exits are not repaired by this
interface; completed writes remain in place.

## Current evidence domain and integration boundary

The current Source17 primary receipt shares the admitted 80-input build
ending `2026-10-09T16:28:29Z`: three existing checks, sixteen whole objects
and eighteen positive public Core roots. This packet replays those current
Source pins and four artifacts as inherited evidence. It does not rebuild,
execute a fixture or claim that this new pair caller is already compiled.
Its own header/body and actual dependency declarations are separately pinned.

The accepted parent uses actual adjacent argument cells, and this helper
requires those actual cells rather than a copied pair. This packet adds no
parent implementation, allocation, ownership, producer, profile dispatch,
Native class or application binding. The complete Root gate, selected PE
replay, ordered Source mapping, literal LEA bytes, bounded current Source
search and staged whitespace check are the candidate's validation. CMake,
ledgers, Ghidra, builds, tests and admission remain unchanged by the worker.
