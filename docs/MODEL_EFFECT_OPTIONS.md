# Model-effect options singleton

Addresses: `0051F6B0`, `0051F770`.

The packet reconstructs the actual eight-byte owner published at `00F8C27C`.
`model_effect_options_singleton_0051f6b0` takes that application's global slot
and its existing `SingletonLifetimeDomain` for `01090AA0`. Its returned pointer
can directly implement `RegisteredModelEffectCallees::call_0051f6b0` and the
model behavior caller's corresponding required getter. It creates no separate
domain, option mirror, wrapper owner, registration list, or success fallback.

| Entry | Native body and ABI | C++ coverage |
| --- | --- | --- |
| `0051F6B0` | `0051F6B0..0051F765`, 182 bytes/49 instructions; cdecl, no inputs, EAX current owner; final `RET` at `0051F765`, one byte | complete established behavior; host CRT and C++ exception boundary |
| `0051F770` | `0051F770..0051F798`, 41 bytes/12 instructions; ECX owner, stack flags, EAX original owner; final `RET 4` at `0051F796`, three bytes | complete established behavior; host free boundary |

Both current Ghidra bodies have zero flow gaps. The primary integrator repaired
an erroneous interior function at `0051F6CA`, whose `SUB ESP,8` follows the
getter's `0051F6C5` global load. This worker made no Ghidra changes.

## Actual layout and caller evidence

| Offset | Storage | Evidence |
| --- | --- | --- |
| `00` | volatile DWORD `native_table_00` | sole table producer `0051F718` writes `00CECA14`; actual table word at `00CECA14` is `0051F770` |
| `04` | volatile byte `option_04` | constructor store at `0051F71E` writes zero; cheat-menu label is `Don't update particles (` at `00CECB20` |
| `05` | volatile byte `option_05` | constructor store at `0051F722` writes zero; cheat-menu label is `Hide particles   (` at `00CECB3C` |
| `06..07` | two untouched bytes | allocation size eight; neither constructor store nor deleting destructor writes these bytes |

All six live getter call references were inspected. These are reads, including
the two cheat-menu label builders; they do not establish a nonzero writer.
The labels and observed gates support suppression interpretations, but field
names remain neutral because the toggle producer and wider policy are unresolved.

| Caller / getter call site | Established consumption |
| --- | --- |
| `0051FA50` / `00520521` | tests `+05` at `00520526`; builds the hide-particles label |
| `0051FA50` / `005205E2` | tests `+04` at `005205E7`; builds the don't-update-particles label |
| `0086B1C0` / `0086B1D9` | nonzero `+04` makes the registered-model factory return null before its allocation |
| `00AF6BE0` / `00AF6BE3` | nonzero `+04` returns AL=1 before examining the owner's completion state |
| `00AF6DD0` / `00AF6E0F` | nonzero `+04` skips its update body via branch `00AF6E18 -> 00AF738B` |
| `00AF73A0` / `00AF73E9` | nonzero `+05` skips the subsequent submission branch via `00AF73F2 -> 00AF7488` |

The direct `00F8C27C` reference set consists of the getter's read/recheck/publish/
register-read/final-read and the scalar's clear. No nonzero flag producer was
found within that reference set; unknown indirect writers are not ruled out.

## Lifetime sequence and service contracts

The fast path captures the current global once and returns it. The cold path
gets the shared manager, captures its `+10` critical section, enters the actual
Win32 section if nonnull, increments its actual `+18` counter, and rechecks the
global. Only a still-empty slot allocates eight bytes and writes the table and
two zero bytes. Placement default initialization preserves bytes `+06/+07`.

Publication precedes the second manager getter. The registration pointer is
reloaded from the global **after** that getter, at `0051F734`, then registered
as the primary pointer. The captured section is released and the global read
again for the return. The getter does not capture a secondary interface or
unregister an owner. The allocator's native null-result branch remains represented,
although the actual `00BF681B` malloc/new-handler loop throws on exhaustion.

| Containing function / call site | Native callee | Actual implementation or contract |
| --- | --- | --- |
| `0051F6B0` / `0051F6D6`, `0051F72F` | `00415350` | existing domain `get_manager_00415350`; shared actual manager |
| `0051F6B0` / `0051F6EF` | `[00CE2218]` | actual `EnterCriticalSection`; subsequent add at `0051F6F5` updates physical counter |
| `0051F6B0` / `0051F70C` | `00BF681B` | `singleton_lifetime_allocate({object,8,8})`; cdecl allocation, `ADD ESP,4` at `0051F711` |
| `0051F6B0` / `0051F73D` | `00BD0C30` | actual manager `register_object`; validate before ignoring null; no retain/duplicate filter/internal lock; native `RET4` |
| `0051F6B0` / `0051F74B` | `[00CE2210]` | actual `LeaveCriticalSection`; preceding add -1 at `0051F746` |
| `0051F770` / `0051F78B` | `00BF65AC` | actual `singleton_lifetime_free`; cdecl pointer, `ADD ESP,4` at `0051F790` |
| `00C6AFA0` / `00C6AFA3` | tail `00411EE0` | native EH dependency: release the captured guard's actual section |

The scalar tests only flag bit zero, clears `00F8C27C` unconditionally even if
it names a different object, restores table `00CE3818`, and conditionally frees.
It preserves bytes `+04..+07`. Assembly saves the input ECX in ESI and restores
EAX from ESI after free; the decompiler's `extraout_EAX` return is misleading.

The native manager `00BD0400` pops the exact registered primary pointer before
dispatching its current virtual slot zero with flag 1. The application's existing
destruction callback must resolve current table `00CECA14`, word zero `0051F770`,
to this scalar. This packet does not install an application-wide callback or
silently accept unknown tables. Calling scalar delete directly while a live
registration remains requires the caller to manage that registration correctly.

## Exception and ABI boundary

Native getter handler `00C6AFA8..00C6AFB1` is ten bytes: `MOV EAX,00D94564`,
then a five-byte tail jump at `00C6AFAD` to `00BF6B43`. At the worker audit it
has no Ghidra function definition; its bytes were decoded directly and matched
against the installed PE. There are no instruction gaps in that raw span.
The FH3 data at `00D94564` has one unwind state, map `00D9455C` containing
`(-1,00C6AFA0)`. The existing eight-byte funclet `00C6AFA0..00C6AFA7` uses
`LEA ECX,[EBP-14h]` then tail-calls `00411EE0` at `00C6AFA3` (five bytes).

Only the section is unwound. Allocation failure leaves the global empty;
registration failure leaves the already-published initialized owner alive and
releases the captured section. C++ RAII reproduces this established cleanup.
It does not reproduce the raw stack guard's table write or native FH3 dispatcher,
CRT handler identity, asynchronous mutation/fault ordering, or original calling
conventions. These are new C++ APIs with actual eight-byte owner storage.

## Verification

The ignored focused fixture `local/model_options_probe_aj.cpp` compares complete
relocated getter/scalar bodies against the source on the real reconstructed
singleton manager, actual Windows critical section and its physical recursion
counter. Known call bindings forward to the actual manager, register and CRT
services; a manager layout carrier exposes the native `+10` section pointer.
Allocation instrumentation only delegates to the actual allocator and fills raw
bytes with `A5`; free instrumentation captures the actual image before actual
free. The original table bytes establish slot-zero dispatch on actual shutdown.

Covered paths: cold initialization, warm reads after modifying both flags,
primary registration/pop/destruction, preserved trailing bytes, null section,
structural null allocation, scalar flags `0/2/3/100h`, unconditional global clear
with an unrelated publication, native ESP balance, and C++ allocation/registration
unwind. Exceptions are deliberately not thrown through the relocated native
handler. The forced-null allocation is structural-branch coverage, not behavior
claimed for exhausted actual CRT allocation.

Validation results, hashes and exact callable rows are recorded in
`reports/model_effect_options.json`. The standalone strict MSVC Win32 compile
includes this newly added source; the existing CMake build is also run but does
not yet include this source because shared build metadata is integrator-owned.
No native original-ABI replacement, game execution or visual validation is claimed.
