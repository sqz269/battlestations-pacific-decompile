# Application movie factory construction failure boundary

The original caller's state `0x21` frees the **captured raw four-byte allocation**. It does not call the factory destructor, unregister it or clear `F8AEFC`. An exceptional registration or sound setup can therefore leave publication/manager residue referring to freed storage. A production terminal retention policy can avoid that host dangling state, but it must be identified as a deliberate departure from original caller FH3 cleanup.

Reviewed main: `2c25911ef30940f93ef3a56c6202198013b59b45`. Read-only dependencies: owner contract `8f14bfd2b64b0f163c997824a0f1d19b4de49ff0`, readiness `857d4234e4a65ce623f166aca3894eeb9194a7d4`, Bink owner `68ab882bbe7709822936c15b21f6e9804076ffb2` and independent review `6d3d02`. These inputs are not activation or runtime proof. This packet changes only this document and its report; no source, ledger, Ghidra state, game, probe, DLL load, build or test was changed/run.

## Caller, frame and unwind entry

`0073D410` is the enclosing Application initialization function: incoming `ECX` is the application owner, two DWORD stack arguments, `RET8` at `0073E52A`. Its prologue pushes state `-1`, handler `00C86CBA` and previous `FS:0`, allocates `12Ch` bytes, then saves `EBX/EBP/ESI/EDI`.

Let entry `ESP=E` and steady caller `ESP=S`. Then `S=E-148h`, the registration record is `E-0Ch`, state `[S+144h]=[E-4]`, and captured allocation `[S+10h]=[E-138h]`. The cleanup's reconstructed parent-frame `EBP=E` selects that same allocation with `[EBP-138h]`. This is a static EH3 frame/ABI derivation, not a dynamic register witness.

Handler `00C86CBA` executes `MOV EAX,00DB609C; JMP00BF6B43` (existing `___CxxFrameHandler3` identity). FuncInfo `00DB609C` has magic `19930522h`, max state `31h`, unwind map `00DB60C0`, zero try blocks and numeric EH flags `1`. State `21h` selects `00DB61C8 =00DB60C0+21h*8`:

| Entry field | Value |
| --- | --- |
| `toState` | `-1`, the sentinel; no cleanup chain to state `20h` |
| action | `00C86BEC` |
| exact entry bytes | `FF FF FF FF EC 6B C8 00` |

The complete action is:

```text
00C86BEC  MOV EAX,[EBP-138h]
00C86BF2  PUSH EAX
00C86BF3  CALL00BF65AC       ; existing _free boundary
00C86BF8  POP ECX            ; balance one cdecl argument
00C86BF9  RET
```

No `00736750`, `00737950`, virtual call, publication store, unregister or Bink/module cleanup appears in this action. Original code/vtable/IAT words remain numeric evidence identities, not callable host pointers.

## Exact construction epochs

`EDI` is zero from `0073D433`. `EBX` is reset to `-1` at `0073D88D` and has no later write before these instructions; its earlier temporary values `3` and `5` are not the movie epoch.

| Native instruction epoch | Caller state and effect |
| --- | --- |
| `0073DF92..0073DF99` | Demote to `-1`, then terrain `F8C218` virtual `+0Ch` call |
| `0073DF9B/0073DF9D` | `PUSH4; CALL00BF681B` (`operator_new` boundary), still state `-1` |
| `0073DFA2..0073DFA7` | Capture `EAX` in `ESI`; restore argument stack; store `ESI` at `[S+10h]` |
| `0073DFAB/0073DFAD/0073DFB8` | Compare `ESI` with zero, arm state `21h`, then null branch to `0073DFD2`; the state store preserves comparison flags |
| `0073DFBA/0073DFBC` | `ECX=ESI`; call registration `007366B0` under state `21h` |
| `0073DFC1/0073DFC2` | Push zero and actual callback thunk `00A4D35C` |
| `0073DFC7/0073DFCD` | Store `CFEA78` to captured owner, then call Bink sound-system thunk `00A4D356`, still state `21h` |
| `0073DFD2/0073DFD4` | Next producer pushes `4`; then writes `EBX=-1` to `[ESP+148h]`, the same state slot after that push |
| `0073DFDB..0073DFEB` | Next allocation occurs outside the movie guard; capture overwritten at `DFE5`, next state `22h` armed at `DFEB` |

The sound result is ignored: any ordinary returned status follows the normal demotion. It is not evidence for sound success or an exceptional cleanup trigger. An exception from the allocation call precedes the movie guard; do not free a capture never acquired or invent a successful null provider. The original null branch remains evidence, while the existing source allocation boundary throws on exhausted allocation.

## Registration failure is not publication rollback

`007366B0` has handler `00C85F90`, FuncInfo `00DB53F0` and two entries at `00DB53E0`:

| State | Predecessor/action | Exact obligation |
| --- | --- | --- |
| `0` | `-1 /00C85F80` | `MOV ECX,[EBP-18h]; JMP00412430`; target only stores `CE3818` to incoming owner |
| `1` | `0 /00C85F88` | `LEA ECX,[EBP-14h]; JMP00411EE0`; guard reads captured section at `+4`, decrements its `+18h` and calls IAT `CE2210` when nonnull; then state `0` runs |

State `0` arms at `007366D0`. The helper writes `CFEA60`, resolves the first manager, captures its `+10h` section, enters it and increments section `+18h`. State `1` arms only at `00736701`, after entry/increment. Publication `F8AEFC=incoming owner` happens at `00736706`; the second manager getter at `0073670C` completes before current publication is reread at `00736711` for registration at `0073671A`. Normal leave also uses the originally captured section.

On a C++ exceptional unwind through the helper, reached guard cleanup/base-profile reset run before the caller raw free. Neither clears publication nor unregisters. After registration has returned, a sound-call unwind has only the caller raw free; it does not reenter the returned helper. Callback replacement publication is not the identity selected by the caller action. Partial registration effects and reached stores remain explicit; no transaction, retry or universal OS-fault recovery is established.

## Normal destruction is a different path

The explicit shutdown row loads current `F8AEFC` at `0073804F`, null-checks, pushes flags `1` at `0073805D` and calls virtual slot zero at **`0073805F`**, after `010909B0` and before `F8C218`.

`00737950` captures incoming `ECX`, calls `00736750`, frees that captured address only when flags bit zero is set, returns the captured address in `EAX` and uses `RET4`. `00736750` writes `CFEA60`, captures/enters the first section, resolves the manager again, unregisters **current** `F8AEFC`, clears that cell unconditionally, leaves the captured section and writes `CE3818` to incoming storage. Incoming owner need not equal current publication. None of those normal unregister/clear actions is supplied by caller state `21h`.

## Smallest source failure and custody contract

1. Keep the exact four-byte captured allocation separate from retained volatile publication and callback-replaceable current publication. Install real `CFEA60/CFEA78` typed deletion bindings before registration. Reuse `construct_movie_factory_00737710`, the canonical manager/captured-section operations and the same retained real Bink tables/callback.
2. If reproducing original caller EH, the exceptional action is captured raw free only and the guard ends before the next allocation. Its possible dangling publication/registration must stay explicit; it is not recoverable successful startup or safe later drain.
3. For production host interruption, enter an explicit terminal whole-aggregate retention path before ordinary destructors/fallback drain release escaped state. Retain owner storage, publication/context, manager/section, module/Impl/tables and all decoder/callback/renderer/native borrowers until OS exit. Retaining the owner instead of native raw free is a deliberate host policy difference, not FH3 equivalence. Preserve reached effects; do not invent clear/unregister/free/retry, sound reset, unload or a returning fallback provider.
4. Normal success still requires real handle close/callback quiescence, explicit factory retirement, then final raw drain before module/table release. Factory deletion alone does not close unrelated decoder handles or prove vendor work has stopped.

Unresolved runtime limits: no dynamic EH/FH3 frame witness, installed Bink callback/ABI or sound result, exceptional registration execution, arbitrary SEH cleanup guarantee, cleanup-failure continuation, playback/clock or gameplay witness. The dangling outcome is a static possible path, not an observed native crash. Root owns actual source integration, surrounding terrain/`010909B0`/full-shutdown providers and production validation. No private accuracy candidate, body count or subtitle guard is changed.

Static installed original PE SHA-256 reverified: `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`. Live target checks remained `C:/Users/sqz269/bsp.gpr` / `/battlestationspacific.exe`; live count `77631`. Fresh live metadata/action bytes agree with on-disk PE bytes. Exact raw spans, file offsets/SHA-256, dependency blob hashes and original export hashes are in [the report](../reports/cc16_movie_factory_startup_eh.json). No new native/vendor-body, ABI, activation or runtime credit is added.
