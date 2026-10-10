# Press-start profile completion owner review

`0067CA40` is a no-argument storage completion callback. It observes the current storage manager when delivered. A nonzero state returns immediately; state zero obtains the menu command screen, dismisses all prompts through that returned receiver, then tail-jumps to title skip. This closes the callback's local owner and ordering question, but does not establish a production menu/game binding. C++ activation remains held.

Fresh Native scope is exactly `0067CA40..0067CA5C`: 29 bytes, eight instructions. Six typed read-only queries verify project `bsp`, program `/battlestationspacific.exe`, x86 and image base `00400000`; configuration selects `C:/Users/sqz269/bsp.gpr`. Complete saved-program bytes match the independently retained original PE. The live prototype names `BSP_PressStartScreen_OnStorageComplete(void)` and bounds this complete body. Two reported data references, `0067CE40` and `0067CE98`, lie in the sign-in handler; the direct-call query reports no callers. Indirect callers are not excluded.

The frozen Source baseline is **`d8d2e5e20`**, before the announced six-file dormant binding integration. Later accepted changes are not adopted or claimed unchanged. All 4,206 Source files were checked against the retained corpus plus the same three explicitly accepted Source3350 overlays: particle manager header/implementation and the GC comment in `game_hosts_script_orders.cpp`. The relied quoted-include closure contains 323 complete files from 26 roots and 590 edges. Angle includes remain external library/SDK dependencies.

The preceding worker `676554f7` and Root acceptance are preserved as complete Git report blobs, with their two documents. Their historical Root proof trees are not recursively embedded or replayed. This review separately retains only the complete Source corpus/overlays, relied whole Source/support files, original PE and accepted finite `007FEFE0` bytes it actually uses. That older body receives byte-identity checking only; all fresh instruction decoding stays within `0067CA40`.

## Complete local behavior and ABI

| Address | Observed instruction / effect |
| --- | --- |
| `0067CA40` | Load current `[0109CECC]` into EAX. |
| `0067CA45` | Compare that manager's `+8h` state with zero. No null guard. |
| `0067CA49` | Branch to the final `RET` for any nonzero state. |
| `0067CA4B` | Call menu command singleton getter `00425D10` only on the zero arm. |
| `0067CA50` | Move the getter's returned EAX to ECX. |
| `0067CA52` | Call `00530650` with that receiver. No null guard. |
| `0067CA57` | Tail-jump to `0068D8A0` after dismissal returns. |
| `0067CA5C` | Plain `RET` for the nonzero arm. |

No incoming register or stack argument is consumed. In particular, this is not a press-start-screen `this` callback, and it does not inherit the profile receiver from the request. The getter result is passed directly as ECX to dismissal. After dismissal the callback performs no register setup for the title tail: EAX/ECX/EDX may have been clobbered by the callee, so none can be assumed to carry a menu, game or profile receiver to `0068D8A0`.

There are no pushes, pops, explicit stack adjustments or explicit memory writes in the body. Under the retained zero-argument-cleanup contracts of the two calls, the nonzero path returns with entry ESP restored and the zero path tail-transfers with the original return address still on the stack. This does not newly prove delegated ABI, exception handling or a meaningful return value. The write count excludes implicit call-stack writes and all delegated/alias effects.

The only absolute memory read is `0109CECC`; the only following dereference is manager `+8h`. There is no direct game global read, menu publication read, game-state write, or access to `F87458`, `E198F8` or `F88958`. The manager observed here can differ from those captured by the request, storage driver or `007FEFE0`. Saving an earlier storage result in a closure would change this current observation.

## Menu lookup, dismissal and title transition

The callback establishes the ordering “current storage state, then menu getter, then dismissal, then title tail.” Exact getter behavior remains delegated. Retained `frontend_states.hpp` describes a `290h` menu-command singleton cached at `E18E6C`, constructed by `00533120` and registered through its embedded lifetime interface at `+0Ch`; `GAME_FRONTEND_STATES.md` records the getter as analyzed but not reconstructed. Those are retained descriptive contracts, not new proof of allocation, publication reloads or lifetime in `00425D10`.

Current `dismiss_all_prompts_00530650` iterates slots 0 through 6 on the supplied screen, calling `dismiss_prompt_00532A20` each time. Dismissal skips empty slots; it suppresses callbacks for Busy or nondismissible records, and otherwise invokes the saved callback with result 2 after clearing record fields but before refresh. Result 2 is also an acceptance result in this subsystem and is not a universal cancellation code. Refresh can restore pending records, and callbacks can reenter storage or menu operations before subsequent dismiss attempts.

Therefore dismissal is not a silent bulk clear. The same menu receiver must survive all seven attempts and their nested work. The final title transfer happens only after that work returns. If reentry changes game or storage publications, the later title routine's own lookup rules decide what it observes; this callback does not capture a game before dismissal.

The detailed retained `MAIN_MENU_PATH` contract and current Source describe `0068D8A0` as:

1. Reset current storage availability unconditionally through `00BD3450`.
2. If a game-state request is already pending, return without releasing the hold byte.
3. Otherwise request state 4 through `004D7920`, then clear the hold at game `+5ECh`.

The earlier press-start summary is less explicit about the pending arm. It must not be read as permission to clear the hold unconditionally. This packet freshly proves only the tail target and its position after dismissal; it does not freshly prove the title routine's exact current-game loads or delegated state-request body. No direct state-5 assignment, interface change or main-menu screen-set raise occurs in `0067CA40`.

## Current production binding and lifetime gaps

At the frozen baseline, `MenuPressStartHost::clear_all_prompt_slots` only logs an unimplemented call. Its `request_main_menu_state` sets `press_start_handoff_requested`; `MenuPathHost` later treats that flag as an input event, and `advance_path` eventually clears it. That frame-driven, deduplicated path does not establish the Native immediate tail boundary after prompt dismissal.

The full-corpus callback identifier query has five matches, all in the press-start header and caller comments; no dedicated `0067CA40` Source entry appears under those identifiers. This is a scoped finding, not a blanket claim about every differently named adapter. The complete named profile-owner query still finds no `ProfileResetState`, `ProfileIoState`, `ProfileSettingsRestoreState` or `PcProfileIoHost` binding in `game_*` Source at this epoch.

The four `MenuPressStartHost` constructions at lines 1554, 1564, 1785 and 1848 of `game_hosts_menu.cpp` are automatic locals. A retained callback cannot borrow one across its caller's return. Its underlying `GameMenuHost::Impl` persists across frames but is eventually deleted before the frontend; that fact alone does not establish survival through pending storage, prompt callbacks, reentry or shutdown.

A future owner binding needs a genuine no-argument completion that resolves current storage at delivery, obtains the correct actual menu receiver only on zero, retains the menu and its services through dismissal/reentry, and then enters the true title-skip boundary. Exact current-game capture/reload and pending-request ownership must be assigned there. Replacing these steps with a cached state, a captured transient adapter, a no-op prompt clear or an early frame-handoff flag would need separate equivalence evidence.

The accepted `007FEFE0` contract remains unchanged: it captures its current game/profile and storage on entry; takes current `F87458` only after reset or archive cleanup/manager notification; and, after settings restore returns, applies settings, commits its captured profile and destroys the outer reader. `0067CA40` may execute synchronously inside settings restore, before that outer tail, or later through a retained settings completion.

Keep three distinct canonical slots: profile callback `F87458`, storage continuation `E198F8`, settings completion `F88958`. This callback does not itself clear or reinstall any of them. Dismissal can invoke prompt callbacks that reenter those owners. Once completion returns, the outer storage driver may still perform its prescribed prompt-slot-0 dismissal. No new retry, request queue, cancellation policy, suppression of reentry or cleanup reorder is supported here.

Lifetimes must cover every remaining access by the captured outer profile/storage owners, current menu and game owners, raw settings/archive readers, Lua/conversion state, prompt/render services and shared slots. The callback contains no retention mechanism or shutdown guard. Exact exclusion or retirement of pending work before owner destruction remains a genuine integration contract; this review does not invent an extra drain or change teardown order.

## Next bounded frontier and replay

The smallest next separately leased Native body is **`0068D8A0`**, to establish the post-dismiss current-game/storage observations, receiver contract, pending-request guard and request/hold order. `00425D10` remains a separate actual menu-publication/lifetime frontier when selecting a raw receiver; `006AD3C0` remains the separate production prompt-response route. The accepted raw profile/settings/archive and shutdown prerequisites are still open. No C++ packet is authorized by this review.

Both validation modes use the frozen `d8d2e5e20` epoch. Portable evidence is self-contained under one directory; copy the report plus all referenced paths under `local/cc12_press_start_profile_completion_owner_review/`, then run from the relocated root:

```powershell
python local/cc12_press_start_profile_completion_owner_review/verify.py
```

The artifact guard permits only that retained directory and this packet's report, and rejects writes, subprocesses, network and new library loads. Replay verifies all pins, corpus/three overlays, complete quoted closure and Source query/excerpt assertions, accepted report Git identities, original PE/saved-program byte identity and all eight fresh instructions. It deliberately does not follow historical artifact pins inside the prior whole reports.

The separate original-worktree check verifies actual Git objects, all current working Source and the exact two owned outputs:

```powershell
python local/cc12_press_start_profile_completion_owner_review/verify.py --original-worktree
```

No C++ edits, compiler/build, tests, Ghidra writes or repairs, other Native-body decoding, Native execution, SDK/OS/game execution or activation occurred. Complete records are in the [machine-readable report](../reports/cc12_press_start_profile_completion_owner_review.json).
