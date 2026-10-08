# Press-start sign-in request readiness

The whole `00A3F3D0` wrapper and whole `00A3F100` target are ready for a small, ordinary C++ reconstruction over admitted actual manager storage. Wiring the current production menu directly to that provider is **not ready**: the application still leaves the online manager publication and its pump null, and neighboring menu queries return neutral values. This audit changes no Source implementation, configuration, native analysis, account state, or SDK state.

The wrapper's current `CG_adjustor_thunk_00a3f3d0` label is misleading. It does **not** adjust the receiver. It writes `DWORD [ECX+28h] = 1` and tail-jumps to `00A3F100`. The target changes a separate field at `+3B0h`; neither body establishes completed sign-in at `+28h = 2`.

## Evidence and scope

The report [cc12_press_start_signin_readiness.json](../reports/cc12_press_start_signin_readiness.json) records complete bytes, every decoded instruction, exact calls and return sites, selected Source fingerprints, the Original executable hash, SDK file pins, and the primary's retained menu log/screenshot. Original image SHA-256 is `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

Both entire bodies were exported from project `bsp`, program `/battlestationspacific.exe`, using `C:/Users/sqz269/bsp.gpr`. Each live CLI query/export verifies project, program, language and image base through `Client.verify()`. The retained exports are under the worker's ignored `local/cc12_press_start_signin_evidence/analysis/`; the matching Original body binaries, live byte captures, static sealing script, Source copies and manifest are retained beside them. Independent PE extraction equals all live Ghidra bytes. Capstone decodes the complete 12/162 bytes without gaps and agrees with every live-listing instruction address.

Only these two native bodies were expanded. Existing downstream Source/ledger contracts and wrapper caller xrefs supply context; no downstream large body, account/network call, game launch, Ghidra mutation, build or test run was added. All 23 pinned Source/header files matched the primary checkout byte-for-byte when sealed. Exact revision and artifact pins are in the report. The Ghidra `undefined(void)` prototypes omit actual inputs and do not override assembly evidence.

## Whole native contract

| Entry | Inclusive body | Size | Native contract |
| --- | --- | ---: | --- |
| `00A3F3D0` | `00A3F3D0..00A3F3DB` | 12 bytes / 2 instructions | `ECX = manager`; one DWORD user/pad at entry `[ESP+4]`; preserve both through tail jump |
| `00A3F100` | `00A3F100..00A3F1A1` | 162 bytes / 46 instructions | `ECX = manager`; one full DWORD user/pad; three normal `RET 4` exits |

The wrapper is exactly `c7412801000000 e924fdffff`. Its store occurs before the target's first external call. There is no ECX adjustment, own return, native vtable dispatch or global-manager reload. The target captures the receiver in ESI and the DWORD user bits in EDI, saves/restores those registers, leaves EBX/EBP untouched, and has no meaningful return value, x87/SSE operation, or EH registration.

The target's sequence is:

1. Push user and call `XUserGetSigninState` at `00A3F109 -> 00A4D572`. This is the existing genuine ordinal-5262 stdcall boundary.
2. Pass its EAX result and user to the existing `004254B0` diagnostic sink at `00A3F115`. That sink has an established bare-`RET` contract. The SDK result controls no branch and is not copied into manager state.
3. Write the user DWORD at manager `+3B4h`. Read the **then-current raw DWORD** at `manager + 8Ch + 4*user`; SDK-side effects therefore precede this read.
4. If that DWORD is zero, read byte `+3E9h`. If the byte is zero, write byte 1 then DWORD `+3B0h = 3`; otherwise preserve the byte and write DWORD `+3B0h = 1`. Each path invokes the diagnostic sink and returns with `RET 4`.
5. If the indexed DWORD is nonzero, capture the current callback identity at `+24h`. If nonzero, call it at `00A3F182` with **ECX = user, no stack arguments**. Write DWORD `+3B0h = 4` **after** the callback returns, invoke the diagnostic sink, then `RET 4`. A null callback still reaches the state-4 store.

Direct diagnostic sites are `00A3F115`, `00A3F14E`, `00A3F16C` and `00A3F195`. The only other direct call is the SDK query; the sixth CALL instruction is the optional register-indirect callback. The three return sites are `00A3F158`, `00A3F176` and `00A3F19F`. The wrapper's transfer at `00A3F3D7` is a JMP, not a CALL. Live xrefs place wrapper calls at `0067D2A9`, `0067D3A1`, `0067D3E4` in the press-start update and `0067CF88` in the invite path. The target's sole observed direct caller is this wrapper.

| Reached storage | Width | Required effect |
| --- | ---: | --- |
| manager `+28h` | DWORD | Wrapper writes request phase 1 before any external call |
| manager `+3B4h` | DWORD | Target writes complete user bits after query/diagnostic |
| manager `+8Ch + 4*user` | DWORD | Read after the external query; zero/nonzero selects branch |
| manager `+3E9h` | byte | Consult only on zero indexed word; write 1 only when initially zero |
| manager `+24h` | DWORD | Capture optional callback only on nonzero indexed word |
| manager `+3B0h` | DWORD | Branch state 3, 1, or 4; state 4 overwrites callback changes after return |

The indexed expression is not proof of a four-element sign-in-state array: `+90h` also begins existing name storage. The provider must preserve the native address expression and require each reached location to be valid, initialized storage for the admitted invocation. It must not clamp user bits, manufacture a user-zero-only rule, assume four initialized status words, or branch on the SDK return instead. Original code has no null-manager or index guard. An ordinary C++ provider can state an admission contract without claiming arbitrary hardware fault equivalence.

The receiver remains the captured ESI identity throughout. External callbacks may mutate that manager; a substituted global reload would be wrong. Native callbacks must preserve the relevant nonvolatile registers and stack balance. Source adapters need an explicit captured callback identity and normal-return lifetime contract. Native fault/unwind behavior is outside a normal-interface reconstruction.

## Existing Source and actual ownership

`GameNativeSettingsProcess::Impl` owns the canonical `F8ABE8` pointer cell, initially null, and the `NativeProfileSettingsContext` that borrows it. `GameStartupHost::SoundServices` borrows that same cell; its platform services and `InputServices` use it too. These are existing actual publication cells, not projected settings values.

SoundServices already owns the selected `XLiveLibrary` and constructs `NativeOnlineSigninRuntime` with that library and the actual frame-clock publication. The runtime forwards ordinal 5262; constructing the adapter does not sign in or create an account. `online_signin_calls()` returns this existing runtime when SoundServices exists. Profile SDK and input/platform bindings likewise borrow that same library and must retain it through online-owner lifetime.

The absence is owner composition, not the absence of all Source constructor code. `construct_native_online_manager_00a40df0` already accepts a real pump/context, constructs the base, writes native profile identity `00D2413C`, and stores its callback arguments at `+20h` and `+24h`. Current application startup does not call it. `game_hosts.cpp` explicitly records the unresolved parent-process named-pipe peer and application IPC owner composition and leaves the online publication null. The SoundServices pump pointer also remains null. `GameNativeOnlineProcess` provides existing CRT/IPC-related services but does not establish that missing application endpoint and manager owner.

`NativeOnlineManagerStorage` is an actual `3F0h` storage type, not a constructor or registration. A zeroed instance, an `OnlineSystemState` projection, or writing a native-looking vtable number does not supply a callable native class profile or real owner. These two request bodies themselves make no virtual-table call, so an ordinary semantic provider need not impersonate a native entry thunk. Production admission still requires the same real constructed manager, its initialized reached fields, and its owning contexts. Native binary entry/table compatibility is a separate unproven claim.

Current `game_hosts_menu.cpp::request_sign_in` discards its pad argument and logs unimplemented. `sign_in_phase()` returns Idle; adjacent user/profile/blocked/invite queries are also neutral. Press-start Source sometimes forwards a selected device pad or invite slot and sometimes explicitly requests user zero; a new binding must preserve whichever call-site value was actually supplied.

## Smallest faithful next provider

Implement ordinary functions for the full wrapper and target, with explicit manager reference, full DWORD user bits, and a narrow request-calls boundary. Candidate descriptive names in the report are hypotheses, not recovered native symbols. Reuse the existing genuine query implementation. Add an explicit `call_callback24(captured_target, user)` boundary; do not route it through `call_callback20`.

The existing runtime only dispatches callback `+20h` identity `00735510`, which sets context value **6**. The installed `+24h` callback identity is `00735520`, and its faithful normal-interface leaf already exists: `set_online_context_four_00735520(user, sdk)` calls `user_set_context(user, 8001h, 4)`. `XLiveApplicationContextAdapter`, borrowing the same genuine library, forwards ordinal 5277 with the existing Win32 stdcall contract. This leaf and adapter can serve the request-specific callback boundary without reconstructing another callback body. Unknown nonzero callback identities require an explicit supplied binding or exclusion from admitted calls. The null identity makes no call.

Prefer a small request-specific interface/adapter composed with the existing query runtime and context-four adapter. Expanding `NativeOnlineSigninCalls` directly would also require reviewing its other consumers. The general `XLiveApplicationCallbackInvoker` includes projected manager/profile dependencies; borrowing those merely to reach context-four would introduce unnecessary and incorrect owner substitution.

The provider must retain the actual manager and borrowed SDK through external calls, capture `+24h` at the observed point, preserve all field/call order, and keep the postcallback state-4 overwrite. Established no-effect diagnostic calls do not need invented logging. The genuine query remains required even though its return is diagnostic-only. No fake SDK result, forced completed phase, detached surrogate manager, callback-six substitution, or UI bypass is justified.

A production host binding additionally needs actual IPC/manager startup and teardown composition, manager and pump publication to the existing shared cells, host access to that same owner, and faithful neighboring menu read/reset methods. Do not turn this bounded provider packet into speculative large owner reconstruction or call account/network SDK functions merely to test readiness.

## Validation boundary

The primary's retained 30-frame injected press-start log reports 145 concrete methods and 77 unimplemented methods, including online owner initialization, six sign-in-phase queries and two sign-in requests. Its `ScreenVisible/state5` summary is menu-state evidence, not account sign-in completion or gameplay equivalence. The accompanying screenshot is pinned as a runtime reference only.

This audit establishes whole-byte, control-flow, ABI and Source-binding readiness. The read-only call report verifier checks the six direct transfer rows (one wrapper JMP and five target CALLs); the register-indirect callback is separately identified and not presented as a verified direct target. No new implementation, native entry ABI compatibility, sign-in success, live online ownership, or gameplay validation is claimed. The misleading wrapper label is recorded for a primary-controlled annotation follow-up; this worker made no Ghidra edit.
