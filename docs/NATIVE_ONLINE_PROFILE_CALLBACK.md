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
