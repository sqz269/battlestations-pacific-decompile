# DamageableClass back-row selection readiness

Owned region: `0087CE88..0087CEBA`, 51 bytes / 15 instructions within 0087CA80.
The CEBB successor and every row field/iteration remain excluded. This packet
adds only this document and its JSON report: no C++, build, test, probe, CMake,
ledger or Ghidra mutation.

The complete parent is still 3,238 bytes / 858 instructions with SHA-256
`00d679920bd1b8f8653e9a0ab1de627f954cfadaf48b8bf0642676fde5162808`.
The selected region hashes to
`0b5b7fb9484dc4c5b30ef97bcf62886907a7d10f11cc22455ecf15f0492442e0`.
Installed parent bytes, every saved/retained-live instruction start and all 51
selected bytes were replayed. Saved and retained-live listing hashes still
match their retained pins. No new Native body, child, data, handler or live query
was opened; all handler evidence is retained from the existing 37-state audit.

## Exact captured and current values

EDI is the actual vector header already established as descriptor+18h before
append. It is the same header identity after row-temporary cleanup. Do not reload
a different descriptor or use append's nonexistent return pointer. Header+0 and
capacity+Ch are not accessed here. The fields used are current begin at +4 and
current end at +8. S=ESP after E4h locals/four saves; S+DCh is the actual writable
four-byte saved-end slot. It is not a copied C++ iterator or return value.

All comparisons are unsigned 32-bit. Subtracting 30h wraps modulo 2^32; it must
not become C++ pointer subtraction outside a proven allocation, signed ordering,
saturation, bounds clamping or a synthetic empty-container result.

| Site | Exact observable action |
| --- | --- |
| CE88 | Capture E0 = current header+8 into ESI, once. |
| CE8B..CE90 | Read current begin. If begin > E0, call BF6713; continue if it returns. |
| CE95 | Derive candidate = (E0-30h) modulo 2^32 in EAX. |
| CE98 | Read current end and save the comparison candidate > that end. |
| CE9B | Publish E0 to actual S+DCh, after the end read/comparison and before branching. |
| CEA2..CEA9 | If the saved comparison was above-end, skip the begin read and call BF6713. Otherwise read current begin; call only if candidate < begin. |
| CEAE | After any second call returns, derive ESI = (E0-30h) modulo 2^32. |
| CEB1..CEB6 | Read current end again. If ESI >= end, call BF6713. |
| CEBB | Continue with that captured-end-derived ESI; no refresh or retry follows a returning third call. |

CE9B is a MOV and preserves CE98 comparison flags for CEA2. A future Source
implementation must save the first comparison result before publishing the slot,
then retain the short-circuit begin read. Moving the publication before the end
read, after the begin read, or after the possible second callback changes the
observed order. It must not recompute the first comparison after the slot write.

On complete normal passage, the header is read four or five times: three end reads, the first begin read and
the conditional second begin read. Each read after a callback observes the then
current field. E0 stays captured throughout. The final selected row remains
E0-30h even if callbacks repair, grow, replace or otherwise change header fields.
No capacity check, retry loop, row dereference, allocation, record write or vector
rollback is present in these 51 bytes.

## Returning or throwing invalid-parameter service

There are at most three sequential calls, at CE90, CEA9 and CEB6. Their conditions
are independent after the specified rereads. Returning does not mean the header
was repaired. Later checks continue once using the observed fields and captured
E0; the final pointer can remain outside the current vector after callbacks.

| Provider outcome | Fragment publication and continuation |
| --- | --- |
| First call throws or never returns | The fragment has not written S+DCh. Any provider writes remain its own effects. |
| Second call throws or never returns | E0 was already written to S+DCh. CEAE has not executed. Provider mutation of that slot is not rolled back. |
| Third call throws or never returns | ESI was already derived from E0; the fragment makes no further slot write. Any prior/provider slot contents remain. |
| Any call returns normally | Preserve its field/slot mutations; perform only the following scheduled reads/checks. Do not retry the completed check. |

If a callback replaces S+DCh after its publication, this fragment does not write
E0 there again. The eventual ESI value still comes from captured E0, not a reread
of that slot. Source could return that raw row address only on normal completion;
it must not publish a successful C++ return value on a throwing path. Original
register state during native unwinding remains a separate ABI boundary.

## Genuine current Source providers

The complete implementation of
`bsp::invoke_native_invalid_parameter_00bf6713()` was inspected in
`src/native_invalid_parameter_noinfo_call.cpp`, not inferred from a declaration.
It is the naked nine-operation schedule: XOR EAX, five zero pushes, the actual
SDK-declared `_invalid_parameter_noinfo()` call, ADD ESP,14h and RET. Its current
primary receipt admits a 17-byte / nine-operation Source body with a real UCRT
import and three existing checks. It takes no receiver or arguments and promises
neither noexcept nor noreturn. The five pushed words are Source API padding.

That is a complete **qualified Source provider**. It does not recreate the
Native BF66EF encoded-global, debugger, handler-selection or Watson policy.
Compatible return, handler state, faults and failure/exception delivery remain
the provider's boundary. No handler is installed or invented in this packet.
The retained primary receipt is not a fresh provider compilation or reader run.

The complete current `invalid(access)` helper in
`src/native_damageable_section_vector.cpp` was also inspected. It calls
`access.invalid_parameters.invalid_parameter(access.invalid_parameters.context)`.
`NativeDamageableSectionVectorAccess` borrows that actual
`SingletonLifetimeCallbacks` service; its callback takes a context, may return
and is not declared noexcept. Current complete append/checked-insert bodies
demonstrate actual calls, current header reloads and continued checks after a
return. They do not install a no-op or treat invalid input as an early return.

The existing zero-argument wrapper is not the context callback's function type.
Do not cast it into that slot or silently replace a supplied service with a new
UCRT policy. No concrete DamageableClass vector-access-to-wrapper binding was
found in the complete exact type/symbol search over recorded Source/header paths.
Other Source adapters and CRT callers exist; the broader captured inventory is
not evidence that all callbacks are absent. The exact binding conclusion is
limited to the recorded paths/patterns and current commit.

## Lifetime and the smallest future Source packet

The row-begin guard has completed successfully and is closed to state 13. This
block does not change that state or construct an object. Actual key S+58h/value
S+2Ch remain in the successful iterator owner; Sections S+80h, Damage S+44h and
Unique S+94h remain in their enclosing owners. A throwing service propagates
through those existing ordinary owners; there is no temporary state-14 cleanup
to retry. The retained native chain is 13->12->11->10->1->-1.

A separately owned Source packet can stay at exactly these 51 bytes. It needs:

- The same actual retained header identity as the completed row-begin fragment,
  the actual S+DCh four-byte slot, and the live state-13 iterator owner/scratch.
- The same genuine returning invalid-parameter service/context used by the
  actual vector access. Its implementation/policy and compatible call contract
  must be explicit; absent bindings must remain held rather than become no-ops.
- Raw 32-bit header reads and cursor publication in the exact order above,
  preserved short-circuit reads and wrapping arithmetic. No slot initialization
  before the first possible call, no copied container and no new ownership guard.
- A normal result carrying the captured-end-derived actual row address for the
  excluded continuation. Do not certify that address as a current valid element
  after a returning repair callback, or rederive it from the current end/slot.
- Whole compiled control-flow/relocation review before Source admission, including
  publication timing on returning/throwing callback paths. No new tests are
  required merely to restate this instruction schedule.

Control flow, storage order and direct Source service bodies are known. Concrete
application callback binding and actual caller/receiver/cursor storage remain
unproved. Existing ordinary service contracts can be borrowed explicitly in a
future Source interface; that does not prove production wiring or Native ABI.
Native FH3/SEH/longjmp, nonvolatile/control-stack/fault identity, full reader,
CEBB fields, next iteration, runtime and gameplay remain held.

The prior Root Source 616 review is retained context: 616 selected inputs,
82 Core + 3 App whole objects, 194 unique positive Core definitions and three
existing checks. This readiness packet ran no build, test, probe or fixture;
historical vector tests and provider checks give it no new reader-execution credit.
