# Native invalid-parameter no-information call: qualified Source candidate

Current Source is registered and build-tested. All nine Native operations map to the17-byte Source body, with a six-byte real UCRT import call. Its five pushed zero words are Source padding; Native handler policy and Original ABI remain unproved.

## Integrator compiled review

Root replayed the candidate Native/Source mapping, actual declarations, repository/dependency pins and relevant excerpts before registration. The normal MSVC Win32 build passed all three existing checks. Root reviewed every emitted operation, complete COFF object and indexed relocation graph, actual Core definition and current map/import context. All twelve prior whole objects remain byte-identical, including Legacy20 code/relocations and eight EH payload/relocation contracts. The existing pending-entity provider whole object, its twelve functions and two44-byte EH sections are also unchanged. No new test, probe, storage, caller, production binding or forced retention was added. The immutable worker report below records its candidate stage; current admission is recorded in the primary report.

Current evidence: `reports/cc12_native_invalid_parameter_noinfo_call_primary_review.json`.

## Worker candidate snapshot

This packet adds the standalone MSVC Win32 entry
`bsp::invoke_native_invalid_parameter_00bf6713()`. Its naked body preserves the
complete nine-operation schedule at Native `00BF6713..00BF6722`, while replacing
the direct Native child call with the actual SDK-declared Source
`_invalid_parameter_noinfo()` API. It takes no receiver or arguments and returns
`void`, without `noexcept` or `noreturn`.

This is an uncompiled Source candidate. CMake registration, whole-object and
indexed-import review, normal build/checks and admission belong to the integrator.
No Source or Original ABI credit is claimed here. The existing
`NativeInputDeviceRuntime::invalid_parameter_00bf6713()` member and the accepted
99-byte assertion / 691-byte parent boundaries remain unbound to this entry.

## Scope and evidence versions

- Worker baseline: published `main` / `origin/main`
  `fef0b981dcd77f0c85490c3ab15e78be046c0791`.
- Owned address: `00BF6713` only. The owned Native body is 16 bytes / 9 operations.
- Accepted Native16 receipt: commit
  `548cbb243055558bab7cb8b56ba6f4789402bb46`,
  [provider ABI audit](CC12_MISSION_LUA_VARIANT_INVALID_PARAMETER_PROVIDER_ABI_READINESS.md).
- Accepted Native36 child receipt: commit
  `17f926287d7fefa069c09410a377dda0994f753d`,
  [five-word target ABI audit](CC12_NATIVE_CRT_FIVE_ZERO_ARGUMENT_TARGET_ABI_READINESS.md).
- Prior Source67 receipt: published revision
  `fbaefa43f0135f71e019bb995c48078c8fa3306f`,
  [primary review](../reports/cc12_native_tick_subnode_base_cleanup_primary_review.json).

The six accepted receipt/document pins, all 67 canonical Source input hashes,
the three existing runtime-provider files outside that set and the four recorded
Source67 artifacts were replayed. The report keeps the receipt's physical hashes
separate from canonical LF hashes where original worktree files used CRLF.
The prior normal build completed successfully at `2026-10-09T15:51:40.392145Z`
with three existing checks. Those artifacts do not include this new candidate.
No prior whole-object or indexed-relocation graph was re-parsed in this packet.

The original installed PE still has SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
A fresh, bounded read of only the owned 16-byte body and an independent x86 decode
match every accepted Native16 byte and instruction. Live Ghidra evidence is
inherited from the pinned accepted audits; there was no fresh Ghidra query or
mutation. Native36, subordinate providers, callers, CRT data/IAT windows and
handler bodies were not reopened. Saved `LIBCRT_unmatched_00bf6713` and
`LIBCRT_unmatched_00bf66ef` names remain unchanged; the new Source name is a
descriptive qualified interface, not a recovered Native symbol.

## Complete operation mapping

Native bytes:
`33 c0 50 50 50 50 50 e8 d0 ff ff ff 83 c4 14 c3`.
SHA-256: `cd349dae43dbf63016fe6ea9339dab15c59041e3224e48cf8a53004224b45e77`.
Native RVA and file offset are both `007F6713`.

| Native address | Native bytes / operation | Source operation | Qualification |
| --- | --- | --- | --- |
| `00BF6713` | `33 C0` / `XOR EAX,EAX` | `xor eax, eax` | Discards incoming EAX and supplies zero for the pushes. |
| `00BF6715` | `50` / `PUSH EAX` | `push eax` | First physical zero word. |
| `00BF6716` | `50` / `PUSH EAX` | `push eax` | Second physical zero word. |
| `00BF6717` | `50` / `PUSH EAX` | `push eax` | Third physical zero word. |
| `00BF6718` | `50` / `PUSH EAX` | `push eax` | Fourth physical zero word. |
| `00BF6719` | `50` / `PUSH EAX` | `push eax` | Fifth physical zero word. |
| `00BF671A` | `E8 D0 FF FF FF` / `CALL 00BF66EF` | `call _invalid_parameter_noinfo` | Actual zero-argument Source CRT API; provider identity and policy differ. |
| `00BF671F` | `83 C4 14` / `ADD ESP,14h` | `add esp, 14h` | Discards the five words after a compatible return; supplies final arithmetic flags. |
| `00BF6722` | `C3` / `RET` | `ret` | Uses the current return slot; no callee argument cleanup. |

Source text contains exactly these nine operations, one call and one plain return,
without a prologue, epilogue, local variable, branch, handler setup or extra call.
The expected normal `/MD` import encoding is a six-byte `FF 15` instruction,
which would make the Source body **17 bytes / 9 operations** rather than Native's
**16 bytes / 9 operations**. This is a prediction for integrator review, not a
compiled size or relocation result. No exact Source addresses, COFF symbol,
import slot, emitted payload or linked retention is established by Source text.

## Stack, register and flag contract

Let `S` be entry ESP, `P` the actual ESP after the Source provider returns and
`R = (P + 20) mod 2^32`. Initially `[S]` is the caller's return slot. The body writes
five zero DWORDs at `S-4`, `S-8`, `S-12`, `S-16`, `S-20`, then CALL writes its Source
continuation address at `S-24`. The called API receives ESP `S-24` and sees the
five words above its return address at offsets `+4..+20`.

These words are unused padding for the declared zero-argument Source API. They
are not five invented C++ arguments, semantic Native expression/function/file/line
fields, or a new typed Native child signature. The provider may alter backing;
the wrapper neither rereads the five words nor requires their values to remain
zero. Initial zero delivery is distinct from immutability during a provider call.

A compatible normal Source return resumes after CALL with `P=S-20`. ADD restores
ESP to `S`, and RET then consumes the current `[S]`, finishing at `S+4`. If the
provider returns with a different ESP, ADD and RET use that actual value; the
wrapper validates or repairs nothing. Writable stack through `S-24`, a usable
caller return slot and any additional provider stack/backing requirements remain
external. A changed return slot changes the eventual destination.

XOR sets EAX to zero, CF/OF/SF to zero, ZF/PF to one and leaves AF undefined.
The pushes and call instruction do not alter those arithmetic flags before
provider execution. The body does not supply a receiver or context: incoming
ECX/EDX/EBX/ESI/EDI/EBP are unchanged at the call instruction. The provider may
clobber EAX/ECX/EDX and flags. On normal return, EAX is provider residue rather
than a defined result; the public return type is `void`.

The owned instructions never write EBX/ESI/EDI/EBP. Whole-entry nonvolatile
preservation still requires a compatible Source provider. No saved local copies
restore provider changes. Final CF/PF/AF/ZF/SF/OF come from `ADD ESP,14h`, using the
actual `P` and result `R`, and survive RET. With balanced return, `R=S`; these are
not fixed zero flags or a replay of the initial XOR. Other flags are not locally
restored. Source code placement changes call/return and fault instruction
addresses. No original exception handler, unwind policy or fault conversion is
introduced, and normal return is conditional on the selected provider returning.

## Current SDK and Source provider boundary

The candidate directly includes `<corecrt.h>` and uses its real declaration:

```cpp
_ACRTIMP_ALT void __cdecl _invalid_parameter_noinfo(void);
```

It is at line 371 of SDK `10.0.26100.0/ucrt/corecrt.h`, whose SHA-256 remains
`822e503b81dd7b3d7df93ca22fced3672a5154484fd42054d5941e619bcf6cbc`.
The inspected integrator `bsp_core.vcxproj` selects `Release|Win32`, toolset `v145`,
`MultiThreadedDLL` and `ExceptionHandling=Sync`. Its hash is
`cbd47dc356edc11bcc281eec4f69ef04e357be34d55decf5a0f2256385a693bc`.
Header lines 132–146 map `_ACRTIMP_ALT` to `_ACRTIMP` by default and normally
select `__declspec(dllimport)` for `_DLL` outside a CRT build. Existing project
selection supports the expected import form, but the candidate's preprocessed
declaration and compiled relocation remain unverified.

The selected declaration has no parameters and is not the adjacent
`_invalid_parameter_noinfo_noreturn` API. Neither the imported declaration nor
this wrapper adds `noexcept` or `noreturn`. No private declaration, alternate
import macro, handler installation, dummy runtime object, captured callback,
return-status translation or catch block is introduced.

The existing production member remains a one-line Source CRT borrower with its
own object API. Its three relevant provider files were hash-replayed unchanged.
That member is not rebound to the new entry. The inherited Source67 import
metadata identifies an `_invalid_parameter_noinfo` import from
`api-ms-win-crt-runtime-l1-1-0.dll` in the existing application. It establishes
neither the candidate's future object/link graph nor a Native provider identity.

## Native36 and remaining integration boundaries

The accepted Native36 audit shows an encoded-global operand at `0109DD64`, a call
to `00C04FDE`, and either an indirect tail transfer through returned EAX or a
second call to `00C04EF3` followed by a tail transfer to `00BF65BB`. Its two tails
retain the caller's five argument positions; provider writes can change their
values. Late stack POPs and path-specific flags remain part of that accepted
Native contract. The value at `0109DD64` is still unread, and no handler selection,
initialization, lifetime, debugger-hook policy, Watson body, concrete return or
failure behavior is proved by this Source wrapper.

Calling the current zero-argument UCRT API does not reproduce that Native global
lookup and five-word tail-delivery boundary. Current CRT handler selection,
return/nonreturn, failures, exceptions, reentrancy and state ownership remain
provider policy. Preserving the outer nine-operation schedule does not close
these differences or establish Original ABI, runtime, startup or gameplay parity.

Integrator review should register only this Source candidate, compile and inspect
the entire emitted object and indexed import edge, verify the actual nine-op
payload and cleanup, run the normal build and relevant existing checks, then make
an explicitly qualified admission decision. Parent691/assert99 binding, Native36
implementation, CRT/data expansion, production-member replacement, Ghidra names
and ledger changes remain separate work. This packet ran no build, test, probe
or runtime execution and modified only its four requested tracked files.

Machine-readable pins, operation mapping and validation boundaries are in the
[Source report](../reports/cc12_native_invalid_parameter_noinfo_call_source.json).
