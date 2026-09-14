# Native online profile callback

`00737D60` is the normal manager+18 callback installed by the startup path at
`0073DC87`, after `00A40DF0` fills manager+20 with `00735510` and manager+24
with `00735520`. It ignores incoming ECX and has no stack arguments (`RET`).
The reconstructed source is an explicit raw-storage interface, not a native ABI
replacement. It borrows the current `00F8ABE8` manager and `00E188A8` game
publications and the application's `NativeStringRawPoolContext`. No projected
`OnlineSystemState`, `ProfileResetState`, or second native pool is used.

| Address | Original ABI | Complete normal behavior |
| --- | --- | --- |
| `00A3EAE0` | ECX=actual manager, EAX=selected-name pointer, RET | Read selected DWORD at manager+11C, shift left seven with DWORD wrap, return manager+90+offset. No callee or range check. |
| `00737D60` | Incoming ECX ignored, no stack args, RET | Reload manager publication, select name, make a temporary eight-byte native string via `0041E870`; reload game publication before display setter and independently before base-name setter; return temporary through actual pool. |
| `007F9340` | ECX=actual profile, one source-header pointer on stack, RET4 | Assign eight-byte display-name header at profile+50. Prefer nonempty display name over profile+3C, take first 31 bytes through `00469840`, then reload current game and copy through NUL to game+1FF0. Return substring through pool. |
| `007F9290` | ECX=actual profile, one source-header pointer on stack, RET4 | Assign base-name header at profile+3C, then use the same display-name preference, substring and current-game mirror schedule. |

The callback captures the game pointer separately for each profile setter. Each
setter holds that profile pointer while its native string helpers run, but
reloads the current game when it writes the +1FF0 mirror. The callback's
temporary and both retained profile fields use the same actual pool. The
profile fields remain owned by the preexisting game/profile owner; this packet
does not synthesize its constructor or destruction. The C++ source requires
live publications and readable NUL-terminated names. Native SEH faults,
asynchronous owner retirement, and binary ABI identity remain outside this
interface. Native Ghidra listing and installed PE bytes were read only.

`reports/native_online_profile_callback.json` records the numeric call sites,
full body spans, bytes, validation and artifact closure. The focused fake-pool
fixture changes the current game publication between setters and verifies the
two retained headers, separate mirrors and pool cleanup. It performs no real
SDK, network, installer, or update-process operation.

## Integrated library evidence

At `708e70c96b97e76e3eca1bde53e339246cb72ea1` all six wave sources are registered once in the default
Win32 target. `scripts/build.ps1` and both existing CTests passed. Four focused
fixture programs were compiled and run against that exact `bsp_core.lib`:
13 copied-original achievement cases, two copied-original reset cases and UI
preimage/cleanup checks, the leaf/profile fake-call fixture, and ten
copied-original pump cases plus dispatcher partial-output/cleanup checks.
Two isolated child runs use fake SDK/Shell calls and actual `_exit(0)`.
The six packet reports have115 checked direct call rows and zero failures.
Fourteen saved names/comments were read back with prior comments preserved;
exports were refreshed. No listing repair was required for these14 bodies.

Immutable checkpoint: `local/checkpoints/708e70c9/native-online-pump-default/validation.json`; SHA256 `e5ad416dc64065a43b0bfe74e4c511c1ffa19de01e71e9a9b983f824e0e03f46`;
3182 artifacts. It pins exact sources, build objects and
libraries, compiler include records, actually searched libraries, mapped
32-bit runtime modules, command/stdout/exit evidence, original byte spans and
three verified worker archives. The worker fixtures retain their original
C++20 harness requirement; the earlier C++17 harness invocation failure is
preserved separately. Production source was unchanged by that harness fix.

The full raw3F0 manager construction/destruction, real SDK asynchronous
lifetimes, client virtual behavior, original ABI/FH3/SEH and gameplay remain
open. The UI's locale and clock-provider boundaries remain as documented.
