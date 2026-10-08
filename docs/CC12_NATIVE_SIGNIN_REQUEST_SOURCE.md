# Native sign-in request Source provider

The whole `00A3F100` sign-in request body and `00A3F3D0` request wrapper now have ordinary C++ providers in `native_online_signin.hpp/.cpp`. A separate `NativeOnlineSigninRequestCalls` interface supplies the existing genuine sign-in-state query and a new captured `+24h` callback boundary. The existing `NativeOnlineSigninCalls` interface and `call_callback20` body are unchanged. There is no production menu binding, manager construction, pump publication, account operation, or Ghidra mutation in this packet.

The full MSVC Win32 Release build and all three existing CTests pass. One ignored fixture linked against the completed production library passes a four-step Source trace and an out-of-domain index rejection. This is ordinary Source behavior and build evidence, not Original entry-ABI equivalence, real SDK/account execution, live sign-in, or gameplay validation.

## Whole Original and implementation contract

The [readiness audit](CC12_PRESS_START_SIGNIN_READINESS.md) established the complete bodies and actual owner boundary. This packet refreshed their live exports and bytes under verified project `bsp`, program `/battlestationspacific.exe`, project file `C:/Users/sqz269/bsp.gpr`. The [Source report](../reports/cc12_native_signin_request_source.json) contains all Original instructions, selected current Source/header fingerprints, production COFF functions and positive relocation targets, archive-member origin, build/test results and fixture limitations. The complete Original 12/162-byte ranges still equal the installed executable and refreshed live Ghidra bytes; Original SHA-256 remains `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

| Original body | Ordinary Source provider | Behavior |
| --- | --- | --- |
| `00A3F3D0..00A3F3DB`, 12 bytes, 2 instructions | `request_native_online_signin_00a3f3d0` | Write DWORD manager `+28h = 1`, then enter the target with unchanged receiver and full user bits |
| `00A3F100..00A3F1A1`, 162 bytes, 46 instructions | `begin_native_online_signin_user_00a3f100` | Query, write selected user, read current indexed word, select latch or captured-callback path, then write branch state |

Original ECX is the actual manager and entry `[ESP+4]` holds the user DWORD. The wrapper changes no receiver address and ends in a JMP; the existing “adjustor” label is incorrect. The target retains the receiver in ESI and user in EDI and exits with `RET 4` at three sites. The new functions expose ordinary C++ references and arguments, not that binary ABI.

The Source schedule preserves all normal-domain field and external-call effects:

1. The wrapper writes DWORD `+28h = 1` before entering the target.
2. The target invokes `user_get_signin_state_00a4d572(user)`. The real runtime forwards existing ordinal 5262. The query return reaches only the established bare-`RET` `004254B0` diagnostic sink in Original; it does not select a branch or populate the cached word. Source discards the value after making the required query and adds no substitute logging or status.
3. After the query returns, write DWORD `+3B4h = user`, then read the current DWORD at `+8Ch + 4*user`.
4. When that indexed word is zero, read byte `+3E9h`. If zero, write byte 1 and DWORD `+3B0h = 3`; otherwise preserve that byte and write DWORD `+3B0h = 1`.
5. When the indexed word is nonzero, capture current DWORD `+24h`. A zero identity skips the callback. Otherwise call the supplied boundary with that captured identity and user, then write DWORD `+3B0h = 4` after return. Callback changes to other fields remain; a callback change to `+3B0h` is overwritten by 4.

There is no global receiver reload or projected `OnlineSystemState` substitution. The indexed word is raw actual storage, not an invented four-status array; neighboring offsets overlap existing name storage. Neither the wrapper's phase 1 nor target state 1/3/4 establishes completed sign-in phase 2.

## Admission and deliberate Source boundaries

Calls require one retained actual `3F0h` manager identity with all reached bytes initialized for the invocation. External calls may mutate it but must return normally and must not retire it. The mathematical DWORD range `8Ch + 4*user .. 8Ch + 4*user + 3` must fit wholly inside that allocation without wrap. In this implementation that storage ceiling is user 216. It is **not** a claim that the real SDK supports 217 user slots, nor permission to treat those bytes as initialized status fields.

For user values greater than 216, the target throws `std::out_of_range` before its query or manager writes. The wrapper has already performed its phase-1 store before entering the target. Original has no corresponding index check: this rejection defines the boundary of the admitted Source interface and is outside the native equivalence claim. No clamping or user substitution occurs.

The admitted captured callback is zero or installed identity `00735520`. The real runtime rejects other identities; there is no arbitrary-address dispatch or fallback. The normal-return contract excludes SDK/callback exceptions and native fault/unwind equivalence. The storage type, Source provider, or native-looking table word does not by itself establish a constructed owner or callable native class profile.

## Genuine callback binding and preserved consumers

`NativeOnlineSigninRuntime` now implements both the old calls interface and the narrow request interface. Its existing query override serves both. Existing refresh/poll Calls implementers gain no new required methods. The old Calls class definition and the complete `call_callback20` function body were compared against the pre-change revision and are unchanged; `+20h` still recognizes `00735510` and forwards context value 6.

The new `call_callback24` accepts `00735520` and invokes the existing `set_online_context_four_00735520(user, callback_context_)` leaf. That leaf invokes `XLiveApplicationContextHost::user_set_context(user, 8001h, 4)`. The member is the existing genuine `XLiveApplicationContextAdapter`; both runtime constructors initialize it from the **same borrowed `XLiveLibrary`** used by the query runtime. The adapter resolves ordinal 5277 and invokes the established void Win32 stdcall interface. Constructing this adapter neither loads another DLL nor executes a sign-in/account SDK call.

The new C++ interface adds a second base and changes the runtime's Source object layout. All affected consumers were rebuilt. This is not a binary-compatible replacement for an older compiled runtime or for Original native manager tables. Current MSVC emits an eight-byte query-interface adjustment thunk for this ordinary C++ multiple inheritance; that compiler artifact is distinct from Original `00A3F3D0`, which has no receiver adjustment.

## Production object and focused validation

The retained production `native_online_signin.obj` is 77,668 bytes, SHA-256 `0d293a0ba19aa9b866fd54389a932c444fa5cc3b7c9d960006f108e1e0abd41f`. A parsed physical member of the completed `bsp_core.lib` is byte-identical to that object. The fixture link map attributes both new entry points to `bsp_core:native_online_signin.obj`. No copied implementation or replacement function definition is linked into the fixture.

The production COFF evidence includes the two new entry bodies, real runtime query and its compiler adjustment thunk, both runtime constructors, preserved `+20h`, new `+24h`, and the existing context-four leaf/adapter/constructor from `xlive_application_callbacks.obj`. Resolved relocation identities connect `+24h` to the real context-four leaf and both runtime constructors to the existing context adapter constructor. Current target/wrapper sections are 163/170 bytes: MSVC inlines the target into the wrapper, retains phase 1 before the query, preserves the selected-user/cache sequence and writes state 4 after the callback. These are current compiler bodies, not byte matches to Original.

The ignored fixture uses defined test-only manager storage and an explicit test-only `NativeOnlineSigninRequestCalls`. It makes no real SDK call. One four-step trace covers zero cache with clear latch, zero cache with nonzero latch, a query-mutated nonzero cache with captured context-four callback, and a direct target invocation with nonzero cache and null callback. It checks:

- Phase 1 is visible to a wrapper query; a direct target invocation preserves its preexisting phase.
- The query sees the previous selected user, while the callback sees the newly stored user.
- Query results deliberately disagree with the cache branch; query changes to both cached word and callback identity are observed afterwards.
- Callback changes to selected user, request phase, callback identity and indexed word remain, while its state-field write is overwritten with 4 after return.
- Four complete `3F0h` byte-image comparisons preserve every unrelated byte. The external event trace is `QQQCQ`, four queries and one callback.
- Direct target input 217 rejects before any fixture query or body storage change.

The probe is a Win32 executable with an embedded manifest. No tracked test suite was added. `./scripts/build.ps1` passed `reconstructed_math`, `native_math_differential` and `tool_tests`; those existing math checks do not imply native sign-in differential coverage. The read-only report verifier passed six Original direct transfer rows, with the optional register-indirect callback explicitly excluded from direct-target proof.

Artifacts, copied selected Source inputs, production COFF, probe source/dependencies/link map, refreshed Original exports, and logs are retained under ignored `local/cc12_native_signin_request_evidence/`, with manifest and archive pins in the report.

## Remaining production boundary

`GameStartupHost` still leaves the canonical actual online-manager and pump publications null while parent-process named-pipe peer and application IPC owner composition remain unresolved. Current menu sign-in/read/reset methods are unchanged. This packet creates no manager, account, SDK state, menu transition or completed sign-in phase. Actual owner lifecycle, same-owner publication and faithful menu bindings must be established before a production sign-in path is exercised. The primary integrator retains annotation and metadata ownership; this worker changes exactly the two Source files plus this document and its report.
