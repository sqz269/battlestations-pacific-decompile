# Native viewport owner

`native_viewport_owner.hpp/.cpp` reconstruct the complete concrete viewport
constructor, deleting destructor and four setters. The owner is the actual
`34h` bytes retained at native camera `+180h`, with its actual reference count
at `+04h`. `NativeViewportView` borrows those fields directly. It holds no second
payload or count, allocates nothing, and must remain stable while a renderer
borrows its address.

Names are descriptive hypotheses. The raw layout is checked for MSVC Win32;
the public C++ functions use new interfaces, and the stored native vtable word
is evidence rather than a callable host vtable. This is not a binary replacement
or a game/render validation claim.

## Recovered functions and storage

| Address / end exclusive | Native ABI | Established behavior |
| --- | --- | --- |
| `00B1F850 / 00B1F8EC` | ECX storage, EAX same address, RET | Construct the concrete viewport, including two live renderer calls. |
| `00B1F8F0 / 00B1F914` | ECX owner, stack flags, EAX original address, RET4 | Concrete then base destructor; ordinary free iff flags bit0. |
| `00B1F920 / 00B1F932` | ECX owner, stack pointer to two DWORDs, RET4 | Forward origin read/store pairs at `+08/+0C`. |
| `00B1F940 / 00B1F952` | Same | Forward dimensions read/store pairs at `+10/+14`. |
| `00B1F750 / 00B1F75E` | ECX owner, stack float32 word, RET4 | Raw MOVSS minimum-depth store at `+18`. |
| `00B1F760 / 00B1F76E` | Same | Raw MOVSS maximum-depth store at `+1C`. |
| `00B1FF60 / 00B1FF67` | ECX concrete renderer, EAX renderer+1A14, RET | Evidence for the required renderer virtual+30 access boundary. |

| Owner offset | Field | Constructor behavior |
| --- | --- | --- |
| `00` | Native vtable word | Base `00CEB130`, then concrete `00D5E5F8`. |
| `04` | Actual interlocked reference count | One. |
| `08/0C` | X/Y | Zero. |
| `10/14` | Width/height | Initially 640/480, then independent live renderer reads. |
| `18` | Minimum-depth float bits | Positive zero. |
| `1C` | Maximum-depth float bits | One raw read of live `00D7A24C`; no FP conversion. |
| `20` | Scissor-enable byte | Allocation preimage during renderer calls; zero only after both return. |
| `21..23` | Padding | Preserve allocation preimage. |
| `24..33` | Scissor rectangle | Preserve allocation preimage. |

There are six owned functions totaling 256 bytes, plus the seven-byte renderer
getter. The audit records all code/data span hashes and original bytes from the
saved `/battlestationspacific.exe` in `C:/Users/sqz269/bsp.gpr`, compared with the
installed executable. Each live batch uses the repository's target-verifying
Ghidra CLI. Worker code did not mutate Ghidra or shared ledgers. The integrator
owns naming, getter definition, annotations and refreshed annotated exports.

## Renderer contract and construction order

Concrete renderer vtable `00D5F0A8+30h` contains `00B1FF60`. Its seven bytes are
`8D 81 14 1A 00 00 C3`: `LEA EAX,[ECX+1A14h]; RET`. Result `+0C/+10` therefore
refers to actual renderer `+1A20/+1A24`. These are separate fields from the
`D3DPRESENT_PARAMETERS` at `+1A28`; the startup code writes the former at
`00B2AF9E/00B2AFA4`. A device's present dimensions are not a valid substitute.

`NativeViewportEnvironment` receives the same live `D3D9StateCache*` publication
used by the renderer, an explicit `NativeViewportRendererAccess`, and a borrowed
reference to the actual live constant word. The current `D3D9StateCache` does not
own the recovered `+1A20/+1A24` storage, so its binding must return references to
the actual parameter fields for the exact captured renderer. A missing or
unsupported binding must raise an explicit error. No private parameter storage,
dimension copy, display-size query or fallback is supplied by this packet.

At `00B1F8AC` the constructor loads global `00F8D394`, loads that renderer's
current virtual+30, calls it and immediately stores returned `+0C` at owner+10.
At `00B1F8C3` it reloads the global, resolves virtual+30 again and stores returned
`+10` at owner+14. A callback may replace the publication, mutate the current
owner, or change the parameter fields. The second callback must observe the
first width store, and its own changes to the owner survive unless subsequently
overwritten by the native height/scissor stores. The two calls are not merged.

Camera construction separately reads height then width on its own captured
renderer. It can use this same access interface without adopting viewport
construction's global-reload schedule. Camera ownership/slot resolution and
renderer binding are integrated by their own packets. In particular, native
renderer `00B26770` uses depth zero/one and does not read owner `+18/+1C`.

## Unwind, deletion and setters

Constructor exception metadata `00DF51E0` has one unwind state. Its map at
`00DF51D8` contains `(-1,00CBCCB0)`; that funclet loads the captured owner from
`[EBP-10h]` and jumps to `00BD30F0`. The handler thunk is `00CBCCB8`. Thus a
failure after the derived initialization restores only base vtable `00CEB130`;
it does not restore dimensions, clear scissor, or free in-place storage.
The C++ initializer's catch performs that cleanup and rethrows. The allocation
wrapper supplies the native caller's ordinary new-expression role and frees its
own allocation if construction throws. It uses the existing shared
`singleton_lifetime_allocate/free` CRT boundary with native and host sizes 34h.

Concrete vtable `00D5E5F8` is `[00BD30E0,00B1F8F0]`. The existing invoker permits
null; a nonnull object calls its deleting destructor with flag1. The concrete
host retain/release helpers use the actual `+04` interlocked count, and the
final-zero path invokes this known leaf profile. They do not provide arbitrary
foreign-subclass dispatch or maintain a side count. A caller handling other
profiles must resolve the actual current virtual method separately.

`00B1F8F0` sets the concrete vtable, calls base `00BD30F0`, tests flags bit0 and
optionally calls CRT free `00BF65AC`. Ghidra's incorrect no-return assumption
omits listing bytes `00B1F90B..00B1F90D`; disk and live bytes are `83 C4 04`
(`ADD ESP,4`). The tail then executes `MOV EAX,ESI; POP ESI; RET4`. It returns
the original address even after free. Flags2 destroys without freeing, just as
flags0 does. The C++ raw object's lifetime ends on either path.

The origin/dimension setters read and store the first DWORD before reading the
second. A source one word before the destination propagates the just-written
word. Two-word snapshot/memmove semantics are incorrect. The C++ helper uses
those exact integer load/store pairs. The depth setters accept raw bits to
preserve signaling NaN payloads and negative zero without argument conversion.

## Verification and limits

The focused local fixture loads only 12 verified code/boundary spans and six
data spans into a sparse reserved image. Uncommitted pages remain inaccessible;
unused committed bytes are INT3. It does not load the PE, execute its entrypoint,
resolve imports, or replace unresolved engine calls with stubs. Original CRT
free and the host's shared CRT free are observed by a hook that inspects the
same owner and calls actual `std::free`. The native concrete seven-byte renderer
getter runs unmodified within a controlled virtual callback; the callback makes
deliberate publication/field changes to test the caller's schedule.

One lifecycle scenario matched all 13 complete 52-byte owner states (676 bytes
per path), with only relocated native vtable addresses normalized. It covers:

- Two lifetimes in the same initially A5-filled allocation, preserving every
  trailing byte and each callback's view of the actual owner.
- Four virtual callbacks, with A-to-B publication replacement before each
  constructor's second call, live referenced fields, and reentrant owner writes.
- A live signaling-NaN constant read on the second lifetime, raw signaling-NaN
  and negative-zero depth setters, and forward-overlap propagation in both pair
  setters.
- Flags2 destruction without free, count 1-to-2-to-1-to-0, and concrete
  vtable+0-to-deleting-destructor dispatch with base vtable visible before one
  actual free on each native/host path.

The host borrowed view's field addresses were checked against the raw owner.
An absent renderer publication raised the explicit binding error and left only
base-vtable cleanup with the initial sizes and scissor preimage intact. The
fixture creates a real D3D9 NULLREF device for the actual `D3D9StateCache`
companions; no viewport is drawn and this is not GPU/render proof.

Strict MSVC Win32 compilation of the new source, real shared allocation
dependency and fixture passed with `/W4 /WX`. Seed verification matched all
eight saved math seeds. `scripts/build.ps1` passed both existing CTest checks
(`reconstructed_math`, `native_math_differential`). The shared CMake source-list
addition belongs to the integrator, so the separate strict fixture compile is
the evidence that this new source was compiled in the worker checkout.

Native C++ exception throwing through the original CRT handler was not executed;
the unwind claim rests on assembly/metadata plus the host failure check. Host
allocation-wrapper failure/free, concurrent replacement, unmasked FP traps,
arbitrary viewport subclasses, installed-game camera ownership and visual output
remain unvalidated. The fixture, preparation script, compiler log and executable
are ignored local artifacts, with paths and hashes preserved in the audit.
