# Profile completion current-owner review

Production profile activation remains unready. The complete `007FEFE0` body resolves the current game and storage manager when the continuation starts, then keeps those captured owners through its local work. It does not receive the profile used by `007FF100`. The existing semantic C++ continuation captures references at request time, so an identity and lifetime binding is still required before those helpers can represent this Native route.

This review owns only `007FEFE0..007FF0F5` (278 bytes, 76 instructions). Fresh typed queries verified project `bsp`, program `/battlestationspacific.exe`, x86 and image base `00400000` for each query; configuration selects `C:/Users/sqz269/bsp.gpr`. The saved project's finite bytes equal the corresponding bytes of the independently retained original PE. No other Native body, table or exception handler was freshly inspected. The single reported xref is the data reference at `007FF15F` in the accepted request function; the direct-call query reports no callers. That does not exclude indirect entry.

The Source baseline is accepted primary `ea95c47b3`. All 4,206 current Source files were compared with the retained complete corpus. Exactly three material changes are explicit accepted Source3350 overlays: `include/bsp/native_particle_model_manager.hpp`, `src/native_particle_model_manager.cpp`, and the GC comment in `src/game_hosts_script_orders.cpp`. Their complete current Git/working bytes and Root's immutable accepted bytes are pinned separately. No old corpus was rewritten. The relied Source closure contains 414 complete files from 42 roots, with 767 resolved quoted-include edges; angle includes remain external SDK/library dependencies, not a claimed compiler closure. Root's complete accepted request review and 2,591 retained files are preserved through strict content-qualified artifact mappings.

## Current owners and exact local order

`007FEFF9` loads `[00E188A8]` into EBX, `007FF000` loads `[0109CECC]` into ESI, and `007FF006` adds `650h` to EBX. The storage state test at `007FF00C` reads `[ESI+8]` exactly once. Neither owner has a null check. No game/profile input argument or request receiver is consumed. The two returns are plain `RET`.

The game and storage globals are not reloaded later in this function. EBX is the captured game's profile for reset, archive read and final commit; ESI remains the captured manager, and EDI becomes its embedded Lua/archive owner at `+38h`. A request's captured manager, the surrounding driver's captured manager and this continuation's freshly captured manager are separate observations. Their equality requires an external invariant; it is not established merely because all originally read `0109CECC`.

| State / stage | Observed order |
| --- | --- |
| Storage state exactly `1` | Call reset `007FDB20` with ECX = captured profile. Only after reset returns, load current `F87458` at `007FF01A`, clear it at `007FF021`, and optionally call the saved pointer. |
| Every other state | Construct a globals temporary and stack reader from captured storage `+38h`; call `007FDF00` with captured profile and that reader. The body itself does not restrict this arm to state `0`. |
| Archive cleanup | Reload buffer `[ESI+30h]`; if nonnull, call `_free`, then clear captured manager `+30h`. Close captured embedded archive through `00B65E80`. |
| Manager notification | Load current `[F8A2FC]` at `007FF094`; if nonnull, resolve its current vtable `+A0h` and invoke it. This manager is observed after archive cleanup. |
| Settings handoff | Only after the optional manager call returns, load current `F87458` at `007FF0A8`, push that value, clear `F87458` at `007FF0B3`, and call `008D7A50` with literal settings receiver `F88980`. |
| Remaining outer work | After `008D7A50` returns, call `008D5B50` with the same literal settings receiver, commit the originally captured profile through `007FAE70`, then destroy the outer stack reader through `00441A20`. |

The saved pseudocode's apparent return from `_free` is incorrect for this body: assembly continues at `007FF083` with caller stack cleanup and the buffer clear. No Ghidra repair was made.

Reset, archive work and manager notification precede the profile callback read. If a delegated call replaces `F87458`, the subsequent local read observes that replacement. Both branches clear before handing off the saved pointer, and neither clears the profile slot again afterward. A callback published by nested work after that clear is therefore not locally erased by a second clear. This is a normal-flow statement; delegated effects and exception paths remain outside the fresh-body proof.

There is no wait or join after settings restore. The retained `008D7A50` Source contract permits immediate callback delivery, synchronous nested storage completion, or return with an interactive storage continuation pending. Consequently, the transferred callback can run before outer apply/commit, or remain pending after the outer reader is destroyed. Do not move callback delivery after commit, delay the outer tail until completion, or add a queue.

## Three slots and surrounding driver state

| Native slot | Existing Source representation | Required relationship |
| --- | --- | --- |
| `F87458` | `ProfileIoState::completion` | One shared profile callback; take the current value at the observed reset/notification boundary. |
| `E198F8` | `StorageOperationState::continuation_00e198f8` | One shared storage continuation across nested driver calls. |
| `F88958` | `ProfileSettingsRestoreState::completion` | Separate settings callback. `PcProfileIoHost` currently holds this state as its own `restore_state_` member. |

There is no direct `E198F8` or `F88958` access in fresh `007FEFE0`. Statements about those slots use the retained driver/settings evidence and current Source, not an expanded Native inspection.

The retained `006ADB50` contract captures a manager before its optional first update, stores its incoming continuation once after that update, and finishes for observed states `0`, `1` or `2`. At completion it takes the current `E198F8`, clears the slot, calls the saved continuation, then dismisses prompt slot 0. If there was no continuation, it does not dismiss. Reentrant work can replace the current slot during earlier host calls, so the invoked continuation need not be the original incoming value.

For an ordinary invocation of `007FEFE0` through that retained driver contract, the driver has cleared `E198F8` before entry. This is not a universal entry precondition recovered from `007FEFE0` alone. A nested settings operation can republish or complete the shared slot. After the outer completion returns, the outer driver still performs its post-callback prompt dismissal, including when nested work created another prompt. Its manager state observation does not prove that the continuation's newly loaded manager has the same state.

Interactive prompts can leave the continuation pending after the original request returns. Retained `006AD3C0` resolves the then-current manager, sets its response, reads the current `E198F8`, and resumes the driver. Production routing for that prompt callback remains an explicit owner dependency. No new prompt policy, cancellation, suppression of reentry or thread-safety claim is introduced.

## Existing Source and durable owner contract

`drive_profile_storage_task` captures `[task, &profile, &settings, &io, &read_host, &write_host]`; `complete_profile_read_007fefe0` operates on those explicit semantic arguments. The complete corpus has no named `ProfileResetState`, `ProfileIoState`, `ProfileSettingsRestoreState` or `PcProfileIoHost` binding in current `game_*` Source. `PcProfileIoHost` mentions remain confined to its own declaration and implementation. This is a scoped Source finding, not proof that every possible differently named adapter is absent.

The four `MenuPressStartHost` constructions at `src/game_hosts_menu.cpp:1554`, `1564`, `1785` and `1848` are automatic locals. Its request hook still logs an unimplemented operation and discards the argument. Retaining one of these adapters would leave a dangling callback reference after its caller returns. Its borrowed `GameMenuHost::Impl` lives across frames, but current teardown deletes the menu before the frontend; borrowing that implementation alone does not prove survival through pending completion, reentry or shutdown.

Existing raw game ownership is relevant but insufficient. Current `NativeGameStorage` construction places the raw `F8h` profile at game `+650h`; `NativePlayerProfileStorage` is byte storage, whereas `ProfileResetState` owns semantic C++ fields. `GameNativeGameRuntime` checks shared construction/lifetime publication cells and retains a raw owner. `GameNativeSettingsProcess` exposes its game publication cell. None of those facts establishes a raw archive-reader adapter or makes a semantic profile reference alias the actual game slot.

Current raw game lifetime Source clears its game publication before later destroying the embedded profile. That ordering is retained Source evidence, not a fresh inspection of the game's destructor or proof that callbacks cannot run during retirement. A newly entering completion requires a valid current game; an already entered completion requires its captured profile to remain alive even if the publication changes.

The smallest complete binding must assign these responsibilities before a production C++ packet is ready:

1. Resolve the current game/profile and storage once at each genuine completion entry, then preserve those captured owners for that invocation. Alternatively, establish a real invariant that request-time references equal every later current observation. The present code does not establish that invariant.
2. Use the same actual `F88980` settings storage and archive reader; a copied settings view or a semantic profile object is not that storage. Keep late mission Lua-machine resolution assigned to the existing settings publication contract.
3. Share all three distinct callback slots across routes and nested calls. Constructing a fresh `PcProfileIoHost` per request would split its settings callback state and would not model Native `F88958`.
4. Retain the game/profile, each captured storage manager/archive, actual settings, conversion flag, reader frames, callback states, writer/output and borrowed services through every remaining access. UI, prompt and rendering owners must also survive the driver's post-callback dismissal.
5. Connect those lifetimes to the real shutdown path. Establish actual exclusion or retirement of pending callbacks before owner destruction; do not invent a cancellation rule, extra drain, new shutdown ordering or process-retention policy.
6. Supply the genuine menu completion and prompt-response routes, plus the delegated reset/archive/manager/settings/content effects. Reuse the existing helpers once their actual owners are assigned; do not activate a shortcut from raw settings startup.

The existing separate `unique_ptr<ReaderFrame>` stacks preserve outer reader objects during nested settings readers. Accepted balanced globals-root teardown after Lua close remains supported; nothing here justifies a reader cleanup reorder or a blanket post-close-use claim. Arbitrary outstanding Lua children/cursors, exceptional cleanup and destruction of borrowed host objects are separate obligations.

## Bounded follow-up and validation

The smallest next Native owner frontier is `0067CA40`, the menu/profile completion supplied by the retained press-start route. It requires a new explicit lease. That review should resolve its current menu/game lookup and state transition; it cannot by itself discharge the raw profile/settings, prompt route and shutdown contracts above. `006AD3C0` and `008D7A50` remain retained contracts unless separately leased for fresh inspection. Optional manager virtual `+A0h`, profile reset/read/commit, settings apply, allocator/archive/reader providers and the SEH handler remain delegated effects.

All 76 fresh instructions were decoded and matched to the retained assembly listing. The audit records every direct/indirect call, branch and explicit memory write: 10 direct calls, 2 indirect calls, 4 conditional branches, 10 pushes and 9 explicit memory writes. The last count excludes implicit stack writes and does not bound delegated or alias effects. Six normal control-flow paths restore entry ESP before plain `RET`, under explicitly declared callee cleanup and nonvolatile-register assumptions. The reader is at entry ESP minus `20h`; the globals temporary is at entry ESP minus `44h`. Exception/unwind behavior and provider ABI compatibility are not established by this stack model.

Run the artifact-only replay from a relocated evidence root containing the report and all referenced relative paths under these three directories:

```text
local/cc12_profile_completion_current_owner_review/
local/cc12_profile_request_production_owner_review/
local/cc12_settings_profile_restore_owner_binding_review/
```

```powershell
python local/cc12_profile_completion_current_owner_review/verify.py
```

The replay checks all pins, the immutable prior corpus and three current overlays, Root's retained proof, complete quoted closure, all current Source query/excerpt results, finite PE/Ghidra byte identity, complete fresh decode and the normal stack model. Its read guard confines opens to retained artifacts and the report, and rejects writes, subprocesses, network and new library loads. Original absolute paths are provenance only. The separately requested live Git/current check is:

```powershell
python local/cc12_profile_completion_current_owner_review/verify.py --original-worktree
```

No C++ edits, compiler/build, tests, Native execution, Ghidra mutations, SDK/OS/game execution or application activation occurred. The machine-readable [report](../reports/cc12_profile_completion_current_owner_review.json) preserves the precise evidence epochs and limitations.
