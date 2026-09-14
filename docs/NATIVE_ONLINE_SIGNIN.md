# Raw online-manager sign-in refresh and debounce

Addresses: `00A3E6A0`, `00A3EBD0`, `00A3F3E0`, `00A3F440`.

`native_online_signin.cpp` implements the four normal native bodies over the
same `NativeOnlineManagerStorage` used by the notification-9 fragment. The
0x3F0 allocation is established by `0073DC50`; `00A3F530` publishes that identity
at `00F8ABE8`. These entry points take the captured manager by reference and do
not replace it with `XLiveSigninState`, `XLiveSystemPumpContext`, a shadow user,
or another singleton. Names are descriptive hypotheses, not recovered symbols.

| Entry | Coverage | Native ABI |
|---|---|---|
| `00A3E6A0` | complete normal body through `00A3E6F4` | ECX manager; no stack arguments; RET |
| `00A3EBD0` | complete normal body through `00A3ED06` | ECX manager; forced byte in one DWORD stack slot; `00A3ED04 RET 4` |
| `00A3F3E0` | complete normal body through `00A3F436` | ECX manager; no stack arguments; RET |
| `00A3F440` | complete normal body through `00A3F494` | ECX manager; no stack arguments; RET |

The public C++ entry points use explicit references, not drop-in register-ABI
wrappers. Required external contracts remain visible in `NativeOnlineSigninCalls`.
The concrete runtime forwards into the borrowed, already-loaded `XLiveLibrary`
and samples the same canonical `FrameClock*` slot as frame/system-time services.
It constructs no DLL, user, manager, clock, listener, or account.

## Storage and producer evidence

`00A40E31..00A40E42` initializes pending `+4`, timestamp ticks `+8/+C`, and
frequency `+10/+14` to `{0,1}`. The four-byte frequency-low field at `+10` is the
actual manager field also observed by other online paths; no separate frequency
cache is introduced. `00A40E48` stores the first constructor argument at `+20`;
the argument is `00735510`, pushed by `0073DC75` before the `0073DC7C` call.

`00A3ECCD..00A3ECF4` produces SDK state `+8C`, all 128 raw name bytes `+90..+10F`,
XUID `+110/+114`, and privilege byte `+118`. The native scratch name is not
initialized. A failed name query overwrites only scratch byte zero after the SDK
returns: any SDK-written tail and any untouched unspecified representations are
copied verbatim. The projected SDK name adapter drops such bytes and is not used.
The comparison follows native case-sensitive equality through a NUL; a provider
with no readable terminator within 128 bytes is outside the bounded C++ contract.

`00A3E6A0` clears `+3BC` only when both `+120` and `+3BC` are nonzero, then writes
zero at `+119/+11A/+2F/+2D/+2E/+120/+31`, one at DWORD `+11C/+3B8` and byte `+2C`,
and `FFFFFFFF` at DWORD `+124`. Other raw bytes survive. These names do not imply
that every selected-user/state field has the same meaning as SDK state `+8C`.

## Calls and ordering

| Caller/site | Native target | Recovered contract |
|---|---|---|
| EBD0 / `00A3EBDE` | `00A4D572` | XUserGetSigninState, user DWORD 0; stdcall; ordinal 5262 |
| EBD0 / `00A3EBF4` | `00A4D566` | XUserGetName, user 0, scratch pointer, capacity 128; stdcall; ordinal 5263 |
| EBD0 / `00A3EC07` | `00A4D56C` | XUserGetXUID, user 0, 8-byte scratch pointer; ordinal 5261; failed status zeros both DWORDs |
| EBD0 / `00A3EC39` | `00A4D482` | XUserCheckPrivilege, user 0, privilege FEh, zero-initialized BOOL pointer; ordinal 5265; failed status resets BOOL |
| EBD0 / `00A3ECAB` | `00A3E6A0` | ECX captured EBP manager, no stack arguments |
| EBD0 / `00A3ECBF` | captured `[manager+20]` | ECX explicitly zero, no stack arguments; installed target `00735510` |
| F3E0 / `00A3F405` | current clock vtable `+20`, `00BEE080` | ECX reloaded from `01090AB0`; sample return-buffer pointer; EAX output; RET4 |
| F3E0 / `00A3F409` | `00530890` | ECX sampled pair, stack difference-output then live manager+8; EAX difference; RET8 |
| F3E0 / `00A3F42D` | `00A3EBD0` | ECX captured ESI manager, forced DWORD 1; pending cleared first |
| F440 / `00A3F44E` | `00A3EBD0` | ECX manager, forced DWORD 1; pending cleared only after return |
| F440 / `00A3F45E` | `00A3EBD0` | ECX manager, forced DWORD 0; pending set after return |
| F440 / `00A3F477` | current clock vtable `+20`, `00BEE080` | ECX reloaded clock; one sample return-buffer pointer; EAX output; RET4 |
| `00735510` / `00735518` | `00A4D3F2` | XUserSetContext, incoming ECX user, context 8001h, value 6; stdcall ordinal 5277; result ignored |

All SDK state/name/XUID queries happen even on the nonforced signed-out return.
Only SDK state 2 queries privilege; it clears manager `+128` when the live `+3B8`
is 0 or 1. State/name comparison uses live manager bytes after the SDK calls.
If `+3B8` is nonzero and SDK state is nonzero, the captured callback executes
before the cache commit; its writes to the subsequently committed cache are
overwritten, while its other effects survive. Missing or throwing required
services cannot silently complete that callback path.

At `00A3F3F5..00A3F404`, manager+8 and the subtraction output are pushed before
the sample output. `00BEE080 RET4` consumes only the sample pointer; `00530890
RET8` consumes the other two. Therefore saved manager+8 is read after sampling,
including mutations performed during that call. Subtraction reuses the existing
modulo-64-bit and signed-rescaling implementation. `FILD/FILD/FDIVP` is spilled
with `FSTP float`, reloaded, and compared by `FCOMIP/JBE`: only ordered float
values strictly above 1 refresh. Equality, rounding down to 1, and NaN do not.
Toggle copies all 16 timestamp bytes, including manager+10, after sampling and
does not rewrite pending a second time if the sampler changed it.

All quartet call sites were checked: `00A4014D` polls captured ESI before the
notification loop; `00A401CF` toggles the same ESI; EBD0's three callers are the
two owned debounce routines. Restart also has the `004D7FEF` tail jump after
`004D7FE6` loads the current `00F8ABE8`; no extra arguments survive that jump.

## Validation and limits

The report records exact input/output hashes and validation results. Twelve live
Ghidra/disk spans total 841 bytes and agree, including all four complete bodies,
timestamp subtraction, clock sample, installed context callback and SDK thunks.
The PE imports independently establish all five ordinals. Raw thunk spans have
inclusive endpoints in the report; no instructions beyond a function are assigned
to its caller. Ghidra was read only; existing provisional names/comments remain.

The single Win32 fixture passed raw SDK name failure-tail preservation, callback
ordering, nonforced signed-out early return, timestamp frequency alias and live
reload after sampling, x87 float rounding to one and unordered NaN, poll/toggle
exception ordering, and restart writes. It linked the strict-compiled production
object with this worktree's `frame_clock.cpp` and `xlive_library.cpp` objects;
the report pins all source, include, linker-library, object and result inputs.
No replacement implementation symbols were linked. The fixture supplies explicit
SDK/clock contract providers and does not count as live XLive execution.

The recovered worktree completed `./scripts/build.ps1` in Release/Win32 with
`/W4 /WX /fp:strict`; both existing CTests passed. Recovery revalidated all 267
saved fixture pins without rerunning the unchanged fixture. The completed build
log is `local/signin-default-build-recovery.log`; the earlier interrupted log is
retained separately. `local/signin-verification-manifest.json` pins the final
source/report, native spans, fixture artifacts, actual build and test libraries,
and installed toolchain files. Compiler binary hashes were captured at recovery;
the original fixture retained its exact commands and include/library logs.

The concrete adapter supports the startup-installed `00735510` callback through
a real DLL call. Other callback identities require a separate explicit Calls
binding. Clock virtual overrides also require a binding: the concrete adapter
supports the recovered `00BEE080` implementation on the shared FrameClock.
On failed QPC, the existing sampler lacks the native uninitialized local output;
the adapter throws rather than manufacturing it. Missing DLL ordinals, missing
clock identity, invalid name bounds and exceptions are explicit C++ boundaries.
There is no claim of hardware-fault/FH3 equivalence, live account execution,
native ABI replacement, full `00A40110`/`00A409F0` event closure, or gameplay
validation. This packet supplies actual-storage prerequisites for that integration.

## Integrated library validation

At `d260af28` this source is registered once in the default Win32 target.
The combined `scripts/build.ps1` build and both existing CTests pass. The
packet's focused fixture was compiled and run against the integrated
`bsp_core.lib`, with saved commands, stdout, exit status, dependency headers
and exact library hashes. The four current packet reports contain 55 checked
numeric call rows and zero failures. Source/header bytes match the reviewed
worker delivery. Fourteen saved names/comments were read back and exported
with previous comments preserved. Exact evidence: `local/checkpoints/d260af28/native-online-procedural-default/validation.json`.
This supersedes earlier worker-specific build or fixture-log limitations;
original ABI/FH3 delivery, full manager/pump adoption and gameplay remain open.
