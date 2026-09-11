# Startup movie player and GUI playback

Addresses: `004f8af0`, `004f8cf0`, `004f8970`, `004f89a0`, `004f89d0`, `004f8a20`,
`004f8ac0`, `004f8c60`, `004f8f00`, `004f8be0`, `004c7ed0`, `004f83b0`,
`00aae320`, `00aafee0`, `00ab0eb0`, `00ab0f90`, `00aac870`, `00aac8e0`,
`00ab04a0`, `00aadf40`. Evidence-only dependencies include `004f71a0`, `004f71d0`,
`00ab0520`, `00ab00c0`, `00aadba0`, `00aae770`, `00aac820`, `00737730`,
`00a4c660`, `00a4c720`, `00a4cd70`, and vtables `00ceae94`, `00ceae7c`,
`00d5c270`, `00d24d98`.

The startup controller is the 34h movie screen published at `00e18d48`. Its
registered front-end slot is **56h**. Reconstructed control flow is in
`include/bsp/movie_player.hpp` and `src/movie_player.cpp`. The screen layout is
34h under MSVC Win32 and its offsets are compile-time checked. The GUI movie
state is a projection; the host still supplies GUI objects, scene geometry,
subtitles/VFS, decoder objects, and OS calls. No codec or renderer was stubbed.

## Playback arguments corrected

`004f8a20` is `__thiscall(screen, NativeString const*, byte prefer_shrink_wide,
float local_z, byte loop)`, `RET 10h`. The four source-language stack slots are
four bytes each; the second and fourth use only their low bytes. The old
two-argument pseudocode loses the mode, mistakes the filename for an x87 input,
and labels stack/register values incorrectly.

The third argument is **local Z, not volume**. Assembly `004f8a6b..004f8a76`
passes it to `00aa7910`, the already recovered GUI local-Z setter. The black
icon receives `float(double(local_z) + 1.0)` at `004f8a93..004f8aac`;
`00d7a210` is the little-endian double `1.0`. The float store is explicit in
the reconstruction. Decoder volume is separately set to `1.0f` during
`00ab0eb0`, through `00a4cbd0`.

The fourth argument is **looping**. `00aae320` stores its byte at movie+105h.
`00ab0f42..00ab0f4f` passes it to decoder virtual +28h. The Bink-backed decoder
constructed by `00737730`/`00a4c660` uses vtable `00d24d98`; that slot is
`00a4c720`, whose raw body writes decoder+0Ah. At the final frame,
`00a4cd70` tests +0Ah: zero sets completed+9h, nonzero calls `BinkGoto(0,0)`.
The name is therefore supported beyond caller constants. The decoder is a
third-party-library boundary; this packet does not reimplement Bink.

## Construction, registration and layout

| Offset | Recovered field or action |
| --- | --- |
| +00h | Front-end vptr `00ceae94`; constructor temporarily uses base `00ceae54` |
| +04h/+05h | Wanted/applied bytes, reusing `FrontEndScreen` |
| +08h | Secondary callback-owner vptr `00ceae7c`; temporary base `00ce3cd4` |
| +0Ch/+10h/+14h | Callback-owner words, zeroed; detailed owner contract remains external |
| +18h | Callback-owner byte, cleared |
| +1Ch | `_FullScreenMovies` page, initially null |
| +20h | Selected movie widget, initially null, retained by stop |
| +24h | `ShrinkWideScreen_Movie` child |
| +28h | `MoviePlay_Movie` child |
| +2Ch | `BlackMovie_Icon` child |
| +30h | Allow-input-skip flag, initially false and cleared by every play |

`004f8af0` does not initialize +24h/+28h/+2Ch or the padding bytes. The native
layout reconstruction preserves that omission. `004f8cf0` first registers the
screen, loads `_FullScreenMovies` with `(1,0)`, hides the page, resolves the
three named children recursively in the table order, and hides the black icon.
The native code assumes successful page/child lookup; the reconstruction adds
no null-success fallback. Page names cross the host boundary as C strings;
the original pooled temporary-string allocation/unwind mechanics are external.

Vtable +00h returns 56h (`004f8b30`); +04h returns true (`004f8b40`), identifying
a screen that manages its own requested visibility. +10h is initialization,
+18h/+1Ch are empty (`004f8940`/`004f8950`), +20h is update, and +24h appends
the page handle to the output child vector (`004f8f00`). `enter_movie_player`
sets both visibility bytes and calls the existing `004f83b0` reconstruction,
using that collected page. The logo caller's final enter virtual is empty.

`004f8be0` restores its derived vtables, releases the page through `00aa31f0`,
destroys the secondary callback owner at this+8 through `00695870`, then calls
base destructor `004f71a0`. The base restores `00ceae54` and scans all 95
registry slots, clearing every alias of this screen. The scalar-delete wrapper
`004f8cd0` is an existing correctly named compiler routine and is not renamed
or reconstructed by this packet. The destructor helper does not free storage.

## Play, stop and skip

Play first clears +30h and hides both movie children. It chooses the shrink
child only when `prefer_shrink_wide != 0` and platform+0Dh is zero; otherwise
it chooses the normal child. The platform flag's broader meaning is not
established. The selected widget receives the copied native string and loop
byte, local Z, and visibility=true. Then the backdrop becomes visible at
Z+1, and the screen's wanted byte becomes true. The filename/loop setter only
stores configuration: the GUI visibility transition triggers decoder start.
`MoviePlayerHost::set_widget_visible` must preserve the generic GUI visibility
propagation and invoke the reconstructed movie activation hook when applicable.

Stop checks only that the selected handle exists, then asks its decoder to
stop running and close if the decoder exists. It hides the black icon and
clears wanted. It does not clear selected, clear applied, hide the selected
widget, or invoke completion. The query is selected != null && decoder != null
&& !decoder.completed; it does not additionally test the decoder's open handle.

The screen update consumes an unused float stack argument (`RET 4`). With no
selected widget it returns. Otherwise input can mark completion when either
`00e198c4` is nonzero or +30h is set, and action 3 or action 1 answers true.
The short-circuit order is retained; on a hit it queries action 1 again and
then action 3 again, marks the decoder completed, and clears subtitles. Every
selected-widget update then calls `00427190(0Dh)` on singleton `004c1e90`.
That event's broader meaning is not named. Skip marks completion and does not
synchronously call the completion handler.

## GUI playback and completion

`004f8970` registers the same plain function pointer on both movie widgets.
Their vtable +7Ch, `00aafee0`, stores it at movie+F8h. The native calls pass no
stack arguments and leave ECX pointing at the widget. The existing logo
trampoline `00685060` and default callback `004f89d0` ignore that register;
the C++ callback type is `void(*)()`. This does not claim compatibility with
an arbitrary native member callback that consumes ECX.

Start `00ab0eb0` does nothing for an empty stored filename. Otherwise it
ensures decoder/render resources, prepares subtitles once, and tries decoder
open with `(filename,1,0)`. Failure presents the original Retry/Cancel OS dialog
(`MessageBoxA` flags 15h): result 2 calls completion if present and returns;
every other result retries open. Success sets immediate volume 1, forwards the
loop byte, prepares the first frame, binds movie textures, and enables running.
Visibility=false uses the same stop/close sequence as `00aac870`.

The GUI update `00aadf40` runs base widget update, binds textures, updates the
decoder, then checks decoder+4h. No open handle means return. An incomplete
movie updates subtitles. A completed movie with no callback returns without
closing. With a callback, it closes first, **rereads +F8h**, and invokes it;
no field is touched afterward. A callback may replace itself or start the next
logo, and close may replace the callback. Caching it before close changes the
native ordering, so the implementation intentionally reloads it.

Default callback `004f89d0` returns in game states 1, 2 or 4. Else it performs
`004c7ed0`: clear game+7184h, disable input context 10h through `00a933f0`,
stop the movie, clear wanted again, and clear game+5ECh. It rereads game state
after these calls and invokes `004cd0f0(0,0,1)` only for states 0Dh/0Fh.
The game-state and cinematic host methods remain explicit; the latter can
forward to the existing cinematic reconstruction. No state value is cached
across the stop sequence.

## Evidence status and remaining integration

Read-only Ghidra queries verified the configured `bsp` project,
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, x86 language and image
base through the query tool's verifier. Existing pseudocode was checked against
assembly for the dropped stack arguments/x87 depth calculation, input query
order, caller pop sizes, page lookup arguments, and decoder start volume.
Undefined bodies were inspected as raw Ghidra bytes and decoded without writing
to Ghidra. The parent integrator owns definitions, annotations, and saving.

Newly named bodies missing at analysis time (inclusive final instruction):

| Start | Final instruction address | Length | End exclusive |
| --- | --- | --- | --- |
| `004f89d0` | `004f8a11` RET | 1 | `004f8a12` |
| `004f8f00` | `004f8f0d` RET 4 | 3 | `004f8f10` |
| `00aafee0` | `00aafeea` RET 4 | 3 | `00aafeed` |
| `00aadf40` | `00aadfa9` RET 4 | 3 | `00aadfac` |
| `00ab0f90` | `00ab0fc9` RET 4 | 3 | `00ab0fcc` |

`00aad560` was also absent, but its body is property parsing, not per-frame
movie update; it is not reconstructed or newly named here. Its final RET 4 is
`00aad5ed`, length 3. `00a4c720` is an evidence-only raw setter ending at
`00a4c727` RET 4, length 3, also not newly named. These findings prevent
incorrectly labelling adjacent code from the enclosing function.

The controller and the listed playback wrappers are reconstructed as complete
normal-path bodies with explicit host boundaries. Native SEH, pooled page-name
temporaries, arbitrary callbacks consuming ECX, actual codec/render resources,
subtitle collections, callback-owner lifetime, application-wide state backing,
and installation/runtime playback remain outside this packet. Matching the
34h data layout does not make the new C++ function signatures native ABI
replacements. Compilation, existing checks and seed byte verification are
reported in `reports/game_movie_player.json`; no visual/game validation is claimed.
