# Native Line GPU prerequisite

This is the GPU half of retained type4 Line. The independent node/lifetime
worker owns raw Line allocation, default/copy constructor, point-vector
providers, destructor and scalar terminal. This packet supplies no page,
Screen, manager, widget allocation, second Model, render queue, projected
polyline or factory arm. Its explicit methods are intended for the SAME page's
retained type implementation once its ownership bridge is available.

## Correct native route

The factory at AA1530 calls AC2380, then AC2D00 (or AC2D60 for copy). The
actual object is104h, pool slot108h, slab index+104h, pool publicationF8BEFC.
Its current table is D5C9E8. AC28D0 destroys its derived vector before AA9730;
AC2CA0 is the scalar deleting destructor and returns the pool slot when flags1.
AC2C50 is inside Flush, not a constructor/destructor entry. ABE310 is inside
Curve's ABE1D0 reader, not a Line constructor. The Line row in the inspected
primary54f3555ef `OCT24_GUI_PAGES.md` already has the correct constructor and
destructor addresses; it needs the GPU/ownership status below, not an address
replacement. No shared page document or type factory was edited here.

Live D5C9E8 table bytes establish +74=AC2430, +78=AC1BB0, +7C=AC2950 and
+80=AC29D0. AC1BB0 is not merely a base78 tail: it calls AA7170, then current50
with the same base color50, then current80. Its inherited current50 is AA6870.

The actual derived vector header begins atEC: proxyEC, firstF0, lastF4, endF8;
FC is committed and100 is capacity. AC2D00 zeros F0/F4/F8/FC and sets100=4,
leaving proxyEC untouched. These fields belong to the sole raw Line authority.
The GPU adapter reads/writes that SAME actual storage. It never creates a
`std::vector` copy or allocates/frees the native point array.

## Concrete GPU composition

`GameNativeGuiLineApplication` exposes:

- `constructed74_00ac2430(owner)`;
- `ensure_capacity_00ac2690(owner, requested)`;
- `flush80_00ac29d0(owner)`;
- `reset7c_00ac2950(owner)`;
- `loaded78_00ac1bb0(owner)`.

It borrows `renderer.game_grid_context().graphics` and its existing raw
declaration/vertex/index factories, actual-owner registry, HAL map/unmap domain,
section-layout services, and the existing numeric effect/material application.
The owner runtime, color service and resources use that same renderer domain.
All owners/providers must survive active calls, retained failures and drain,
preserving the Line profile and its same-owner base association through callbacks.
Cold guidefault acquisition requires the SAME existing open shader-cache
bracket; this application creates no second shader cache.

AC2430 composes the real B73B60/B73D70 mesh and B75170 association/creator
release; `SimpleColor.mvfm`; renderer38, renderer5C(4,1,declaration),
renderer60(8,1,65h); real B73BB0/B73B70 setters; actual533FA0 section;
`guidefault.mshd` through535320; section primitive2 and start ranges0;
B864C0, creator release, B865A0 and B73C60; material texture0; and native index,
vertex, declaration, section creator releases in that order.

AC260D's 004C12B0 getter / AC2612 manager+28 load is replaced by one explicit
`initial_manager_texture_28` source service. The provider must observe the
completed Step2 operation0 with manager_offset28, returning that exact result
while retaining the same resource app. Nonnull results require reference_owned,
D61948, the actual+04 alias and canonical owner domain. A legitimate null is
passed unchanged to B189F0, as the original AC2612..AC261A does without a test.
The current installed whiteGui guard rejection produces this null; the genuine
material setter already admits null and adjusts its slot count before identity
comparison. No substitute texture is selected. This source correction is not
a native Line runtime claim.
It constructs no manager and invents no substitute texture/global/publication.

AC2690 stores max(request,4) into actual100 BEFORE the geometry/factory calls.
A changed capacity creates2*cap vertices and2*cap indices with the original
flags/format. It maps old then new, copies min(actualFC,actual100) vertices using
the STILL retained declaration's currentCC stride, unlocks old then new,
publishes the vertex stream, then repeats the native index path with live
counter reloads. It rebuilds section0's actual layout and releases the index,
vertex and declaration creators. DWORD count/byte products retain32-bit wrap.
This replaces no real resource with a capacity integer or host byte-vector.

AC29D0 maps the actual index range `(n-FC)*2` at `FC*2`, then the actual vertex
range `n-FC` atFC. Section0 gets n vertices / n-1 primitives. Its unusual native
index address begins at map-2: each reached segment writes the previous/current
WORD at map+4*k-4 / map+4*k-2. The first uncommitted point skips those writes.
Points are read from the vector's front at k, even when the destination begins
at committed. Positions/UV/packed-or-float white color use the SAME raw mapped
stream's semantic offsets. X and Y preserve the inspected two-pass x87
spill/reload order; Y multiplies the live E12FF4 cell before subtracting this
owner's pivot. The established owner/layout supplies its SAME live pivot/color
fields through the source interface, rather than a copied base widget.

After index then vertex unlock, FC becomes the captured n; valid full-range
erase sets actualF4=actualF0 while retaining EC/F0/F8. Layout rebuilding follows.
The native behavior of clearing pending points while retaining FC is preserved.
Reset7C clears the SAME vector, zerosFC, ensures capacity4, then zeros section0
vertex/primitive counts. Loaded78 performs baseAA7170, AA6870(current owner
color), then this actual Flush80 path.

## Identity, lifetime and integration contract

The binding's `actual_identity(owner)` returns
`GameNativeGuiLineIdentity{same_owner, raw_line, canonical_reference, page_token}`.
The token's get() must equal &owner and keep its sole layout/raw allocation live.
Before GPU effects the adapter checks type4/D5C9E8, canonical raw+04 reference
and owner registration, SAME GuiWidgetOwnerRuntime, and raw Line4C equal to the
owner's SAME live actual Model. The raw authority must invoke its AA6720 model
binding before parenting/current74; the GPU adapter does not create that Model.

Every entered operation has one stable retained frame containing names,
acquired declarations/streams/section/material and mapping phases. Exceptions
preserve partial effects and retain the complete graph; no retry, rollback,
automatic unlock, creator cleanup or native scalar transport is attempted.
Completed frames can be replaced; no per-frame resource/history list grows.

After genuine queued/model/scene retirement, the page authority calls
`release_line_borrow_guard(owner)` while raw Line remains live and raw+04=1.
Then `release_retired_line(layout)` forgets only GPU metadata. Its one actual
Line destructor/scalar/vector/pool transport remains with the raw authority.
No type implementation is installed or native widget deleted by this app.

Remaining integration is explicit: a sole-page factory bridge and authoritative
current4/AA9730 transport must connect the node worker's actual allocation to
these callbacks. The independent raw provider now supplies qualified stock
vector copy/append; it does not admit Line in GuiWidgetOwnerRuntime. The shared
base-profile gates also need either verified Line admission or explicit Line
type overrides: current4C=AA6980,54=AA68F0,58=AA7970,5C=A9E110 and70=AAA3E0
are present in the live D5C9E8 table. The GPU code cannot supply this bridge by
cloning a page or enabling the existing projected Line kernel.

## Evidence and validation

The configured project is `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`; `ghidra ensure --status` confirmed the live program
before the batch. Existing source/export searches preceded recovery. Full
assembly was inspected for AC2430, AC2690 and AC29D0, including the x87 writer,
register-sensitive copy ranges and current78 tail. No re-import or game install
mutation occurred. Existing vendor vector/CRT bodies are bindings, not game
reconstruction credit.

This is a new Win32 C++ service interface over established actual resource
bodies, not a binary widget replacement. It supports canonical valid vector
extents, nonnegative resize copy counts, valid semantic offsets and reached HAL
mappings. Malformed-vector returning CRT handlers, original stack/FH3/SEH/fault
transport, original allocator frame/preimage identity and runtime rendering
remain separate proof boundaries. Missing prerequisites fail explicitly and
retain entered effects rather than manufacture successful draws.

`scripts/build.ps1` completed Win32 Release and linked bsp_game.exe. Both
existing CTests passed: reconstructed_math (0.34s), tool_tests (12.89s), 2/2
overall (13.29s). Evidence is `local/oct24_gui_line_build3.log`; the Line TU compiled in
the preceding incremental build, while this final build supplied the primary
baseline's missing GUI Render/Node registration. The initial compile found and
fixed one missing owning-header include. These checks execute no Line GPU path.

Temporary local CMake admission is restored exactly before commit. Final source
registration and page/factory activation belong to the integrator. No runtime
switch, new tests or game run is added. Construction, actual Line rendering,
buffer-resize behavior and teardown remain unverified at runtime.

Existing Ghidra names/comments were retained and evidence appended through
`ghidra_annotate.py --apply`'s write lock. Old annotations are recorded in
`local/ghidra-annotations-20261024T093921Z.json`; the project was saved and all
five affected exports refreshed. Corresponding name/source ledger records are
in the AC0000 shards, retaining older reconstruction records.
