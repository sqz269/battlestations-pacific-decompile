# Original compiler sampler-entry word (CC10)

This read-only audit narrows the unresolved first source0 input documented in
[R98](NATIVE_SAMPLER_STACK_LIVENESS_R98.md). It establishes an upstream write,
the later clobber boundaries, an exact original-frame capture location, and
the shared physical word across both compiler calls. It does **not** establish
the first word's final value or connect a guessed source preimage.

## Stack relationship

Let `Q` be `B3C3A0`'s steady ESP after its saved EBX/ESI/EDI. The three argument
pushes at `B3C45C/B3C464/B3C46F` and call at `B3C47B` give compiler entry
`T = Q-10h`. `B3B3C0` pushes its three EH words, allocates `C0h` locals and
saves four registers; its steady ESP is `P = T-DCh = Q-ECh`.

Both calls `B3C018` and `B3C024` push two arguments from the same `P`. The
sampler entry ESP is `P-0Ch`; the nonempty loop then saves five words, giving
`B = P-20h`. The required `B-10h` DWORD is therefore exactly:

`B-10h = P-30h = T-10Ch = Q-11Ch`.

At `B3B328`, the two reference arguments are already pushed. `B5F100`'s
`SUB ESP,8; PUSH ESI` then places its byte store at `[ESP+8] = P-30h`.
It writes only the stage byte and later copies the complete DWORD at
`[ESP+0Ch]` after its additional EDI push. Native pass-copy behavior still
requires preserving the other three bytes.

## Genuine producer, followed by unresolved calls

`B3BFB6` calls `B5F9B0` with one argument. In that constructor, the EH pushes
and `SUB ESP,18h` bring ESP to `P-2Ch`; `B5F9C8 PUSH EBX` writes the actual
caller's EBX bits to `P-30h`. This is a genuine upstream write, not an inferred
zero or return PC. The saved-register slot remains protected until the
constructor restores EBX at `B5FAE2` and returns.

It is **not an admitted final sampler preimage**. These later calls occur
before `B3C018`:

| Site | Current call | Why saved EBX is insufficient |
| --- | --- | --- |
| `B3BFD7` | COM pixel shader's current table `+8` (`Release`) | External implementation may write the below-ESP word. No original runtime target/body/stack capture was obtained. |
| `B3BFDE` | `B5F080` pixel shader setter | Its own direct pushes reach only `P-14h`, but it calls current increment/decrement imports and possibly a captured old owner's current virtual `0`. |
| `B3BFE7` | Current `CE2220` decrement | The exact current original-process import target has not been captured. |
| `B3BFF7` | Current pixel-owner virtual `0`, if decrement returns zero | Terminal call can write the word. This branch cannot be removed by assuming nominal counts. |

The existing fresh pass base writes `+58=0` at `B5F781`, but that alone does
not prove every later callback/current-field invariant needed to remove the
setter's old-owner release path. The audit keeps all actual branches.

The local **disk** `SysWOW64/kernel32.dll` exports for increment/decrement were
also read. Both are short x86 bodies with one `PUSH EBP`, no local allocation
and `RET4`; on the setter import path this would write only as low as `P-18h`.
That is conditional disk evidence, not proof of the current original game's
IAT binding, absence of hooks, or the COM `Release` implementation. It neither
supplies a residue nor closes the external call boundary.

## Smallest valid capture contract

Capture the original DWORD at `B3BFF9`, after all preceding calls have returned:
`[original ESP-30h]`. Equivalently, immediately before original `B3C018`, read
`[original ESP-28h]`; its two pushed arguments are then `[ESP]=pass` and
`[ESP+4]=root descriptor`, with `ECX=builder`. Record the original process,
thread, executable bytes/base, compiler-frame ESP, builder, pass and descriptor
identities together with the exact four bytes.

This is an evidence contract, not a capture implementation. No debugger was
launched or attached; no native process was stopped or modified. Reading a
source C++ compiler-private stack, transplanting a relocated helper's return
PC, observing an unrelated invocation, or declaring the saved EBX still live
does not satisfy it. A future observation proves that invocation's input;
it does not automatically establish a constant for other calls or systems.

## Second sampler call

After a normal first sampler return, its epilogue only restores registers and
executes `RET8`. The caller's `B3C01D..B3C024` sequence loads the current mode
descriptor, pushes it and the same pass, then calls the same sampler. Neither
sequence writes `P-30h`. The second entry therefore uses the **same physical
word**, not an independent native allocation.

The first call's exact word and R98 known/unknown state may consequently carry
into the second call when both refer to this proven original frame. A final
sampler-state call can establish its native return PC; an uncaptured source1
release can leave knowledge unknown. An empty first descriptor leaves the
word unchanged and does not establish missing initial bytes. Do not reset
knowledge or transfer it across unrelated compiler invocations. Existing
source contexts remain unchanged by this audit.

## Evidence and limits

Six complete bodies have **5,089 live Ghidra bytes equal to the installed PE**:
`B3C3A0` (274), `B3B3C0` (4,049), `B3B280` (306), `B5F9B0` (320),
`B5F080` (59), and `B5F100` (81). The report records complete-body hashes,
inclusive endpoints and final instruction lengths. Six distinct direct
call-site rows pass live containment/target/instruction verification. Three
external/current-target rows are explicitly indirect and remain unverified.

Raw listings, live bytes, disk-import data and scripts remain ignored and
SHA-indexed by [the report](../reports/native_sampler_compiler_entry_cc10.json).
The initial capped compiler-byte output and its preserved complete spill are
both retained; the full 4,049-byte spill supplied verification. The earlier
readiness verifier counted the setter twice (seven passing rows); the final
report deduplicates it to six distinct sites.

No source/Ghidra mutation, new native function, native probe, build requirement
or runtime claim is introduced by this documentation packet. Installed normal
descriptors and cached shader use retain R98's first-source0 requirement.
No `B107F0` activation, bloom extent change, masked DWORD, fabricated padding,
native ABI/EH claim or full compiler admission follows from this audit.
