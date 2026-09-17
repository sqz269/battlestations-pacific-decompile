# Native game constructor parent, R105

Addresses: `004DDB90`; analyzed caller in `0073D410` at `0073E150..0073E1A9`.

R105 reconstructs the complete **normal parent schedule** of `004DDB90` over
required external services. It provides raw game storage and the construction
sequence needed before actual game input and online-profile startup can be
composed. It does not yet create the application game owner: the external
subsystem providers, destructor and native exception cleanup remain open.

Files: `include/bsp/native_game_construction.hpp`,
`src/native_game_construction.cpp`, `reports/native_game_construction_r105.json`.

## Evidence and coverage correction

The existing `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe` was verified.
The full parent `004DDB90..004DE266` is 1,751 bytes with no listing gaps. The
90-byte caller fragment, nine image DWORD constants and initial game publication
bring live-Ghidra/installed-PE comparison to 1,881 bytes. The parent has 37 direct
CALL instructions; the caller fragment has four. Callback addresses passed to
the CRT array helper are arguments, not additional direct CALL sites.

The previous reconstruction ledger called `dyn_world_descriptor_004ddb90` a
function even though `src/dyn_world_settings.cpp` explicitly implements only the
interior world-descriptor block. R105 preserves that helper and its evidence,
reclassifies its ledger record as a fragment, and records the full normal parent
separately. This is **zero new unique function addresses** in the ledger count.
The previous heuristic Ghidra name `CG_array_ctor_helper_004ddb90` is replaced
by `BSP_Game_ConstructActualStorage`, a descriptive hypothesis, not a recovered
symbol. Prior names/comments/ledger values are retained in the report and local
annotation receipts; prior comments are preserved, the project saved and exports
refreshed.

## Native contract

The caller allocates `71A0h` bytes, unconditionally clears them with `memset`,
then tests the allocation before constructing an actual temporary string header
and invoking `004DDB90`. R105 analyzes this caller; it does not implement the
parent allocation or change the original null-fault behavior. Constructor ABI:
ECX is the game storage, one stack argument points to an 8-byte name header,
EAX returns the same game and the native epilogue is `RET 4`.

`NativeGameStorage` has that observed extent and no implicit byte initialization.
The constructor preserves all unlisted bytes. It reuses the existing genuine raw
input configuration at game+`3C`, raw Lua state at game+`1A0C`, and raw string
resize/copy primitives. The input name header can alias game+`7164`; that header
is cleared before the original self-copy comparison, which the source preserves.

Other reached bodies are required `NativeGameConstructionCalls` services, named
by their native addresses. There is no default production provider or success
fallback. CRT array iteration and STL allocation remain library contracts.
Array callback `004D3730` currently lacks a Ghidra function and remains an
unreconstructed dependency. The participant/profile/race/embedded constructors,
global settings/configuration/resource parsing, timed allocations, mission Lua,
Dyn engine/world and critical-section services still need actual composition.

The source preserves sentinel reloads, byte/DWORD stores, array callback IDs,
five ordinary allocation null branches, three late global allocations and the
movie publication clear. Actual game publication `00E188A8` occurs at `004DE105`,
before Dyn initialization. Subsequent member stores retain the captured game even
if a called service replaces the current publication. The 64-byte descriptor is
fully overwritten from borrowed image bits in native order before its consumer;
it does not use the old helper's hardcoded constants. The first timed argument
uses x87 `FLDZ/FSTP`; the other two use `FLD/FSTP` from borrowed image values.

The operation records the native call site, member-unwind state and the native
tracked allocation. Exceptions retain the partial graph and reject replay. A
failed/running operation cannot silently destruct; only a diagnostic owner that
has resolved its graph can acknowledge cleanup. This is an admission/lifetime
constraint, not original FH3 cleanup or a production recovery path. No binary
entry replacement, private-stack alias equivalence, special x87 status behavior
or actual game destructor is claimed.

## Validation

- Strict MSVC Win32 build and all three existing CTests pass. No permanent test
  suite was added. Initial CMake registration and inline-assembly syntax failures
  were corrected; their logs are retained.
- A focused `/MD /W4 /WX /fp:strict` manifested probe executes copied original
  parent bytes and the source with the same required external boundaries. Four
  cases match 135 ordered observations and 6,144,440 bytes across four binary
  pairs: zeroed caller allocation, patterned storage with self-name alias,
  patterned storage with all five ordinary allocations null, and replacement of
  the game publication during Dyn initialization.
- Each boundary captures the entire `71A0h` game, five supplied publication
  DWORDs, the `4000h` allocation arena and semantic arguments; the world boundary
  additionally captures all 64 descriptor bytes. Private-stack pointers are
  normalized to their contents. Genuine current input/Lua primitives are shared
  by both lanes; string resize uses its genuine source with a controlled allocator.
  External subsystem bodies are explicitly controlled, not independently tested
  native callees. Native exception handling is not executed by this fixture.
- A source-only late failure at `C55F50` retains publication, exact call stage
  and unwind state and rejects replay before diagnostic retirement.
- The ordinary application smoke **failed** with `C0000005` in the existing
  renderer cached-state call through a null device. A debugger captured both
  first- and second-chance failures. An independent D3D9 probe returned
  `8876086A` (`D3DERR_NOTAVAILABLE`) for device creation and capabilities. The
  application machine-code section, data, resources and relocations match the
  previously passing R104 executable; all 53 application objects are identical.
  Only two timestamp fields differ in the executable. This is consistent with
  the previously observed renderer failure in `NATIVE_RENDERER_APPLICATION_R69.md`;
  the exact display/backend transition is unknown. No production graphics change
  was made. This run does not reach the new constructor and provides no successful
  current application or game-construction runtime proof.

Tested sources, dependencies, binaries, fixture images, native evidence, failure
logs and annotation receipts are sealed before the integration rebuild. The
report records the combined integration build separately; file/build/fixture
evidence is not game or ABI validation.

## Follow-up work

Recover the actual array-element and embedded constructors and their destruction
contracts; bind real profile, configuration, resource and Dyn providers. Then
compose the native game allocation/lifetime with the existing raw online and
input-action owners. `004E4A40` frame input services and actual mouse binding
remain unconnected. Full FH3 cleanup, startup, gameplay and visual proof remain
open. Do not introduce a fabricated game clock or profile owner to bypass them.
