# Initial baseline — 2026-09-08

The repository was empty and was initialized on branch `main`. Work uses the existing
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, through the Ghidra MCP
server's HTTP backend at `127.0.0.1:8089`.

## Target and inventory

- PE32, x86, Windows compiler specification, image base `0x00400000`.
- Disk entry point `0x00bfd2bd`, matching the saved project's `entry` export.
- Disk size: 12,223,752 bytes.
- Disk SHA-256: `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
- Saved project: 62,514 total functions, 138,546 symbols, 752 data types.
- Internal-function inventory: 62,072 unique addresses; 441 classified as thunks.
- 33,023 internal functions retain `FUN_` names. Other names include library matches and
  saved annotations; a descriptive name does not prove an original symbol was recovered.
- External-symbol export: 443 entries. This is a symbol count, not a function count.
- Initial pseudocode + assembly export: 16 functions.

The count difference is expected from the server APIs: `ListingService.listFunctionsEnhanced`
uses Ghidra `FunctionManager.getFunctions(true)` (internal iterator), while program metadata
uses `getFunctionCount()`. They must not share the same completion denominator.

The five reconstructed math functions' full byte ranges match the current disk PE;
see `reports/seed_byte_comparison.json`. Whole-image identity has not been checked. The saved
analysis has a much larger function inventory than the fresh import attempted earlier, so the
saved project remains the analysis source. No packing or corruption conclusion follows from
the fresh import's low function count alone.

## Reconstructed functions

| Address | Interpreted behavior | Original interface observed in assembly |
| --- | --- | --- |
| `00401170` | Float absolute value | Stack float, `RET 4`, result in x87 ST0 |
| `00401c20` | Double vec3 `b cross a` | Output EAX, input a ECX, input b EDX |
| `00401cb0` | Double vec3 `b - a` | Output EAX, input a ECX, input b EDX |
| `00401cd0` | In-place double vec3 scale | Vector EAX, stack double, `RET 8` |
| `00401cf0` | Double vec3 squared length | Vector EAX, x87 ST0 result |

The C++ functions use ordinary explicit arguments. They do not implement the original ABI.
Vector writes preserve original order, including aliasing behavior. The squared-length
assembly adds `y*y + x*x` before `z*z`; its pseudocode had reassociated that expression.
The absolute-value routine explicitly rounds to float before reloading its result.

The original vector arithmetic uses x87. Modern MSVC code generation and the C++ `double`
return can differ in intermediate precision, NaN payloads, exceptions, and rounding behavior.
Bounded exactly representable fixtures do not settle those differences.

## Startup and dependencies

Observed startup path:

`entry 00bfd2bd` → `___security_init_cookie 00c1815e` →
`___tmainCRTStartup 00bfd0dd` → `FUN_008f81f0`.

The CRT routine is labelled by Ghidra as a Visual Studio 2005 Release library match. The current
build uses MSVC 19.51 as a practical reconstruction toolchain, not a binary-matching compiler.
`008f81f0` is a likely WinMain based on its four-argument CRT call site. Its current decompiler
prototype incorrectly shows no arguments, so confirm the ABI before reconstructing it.
Its body includes COM setup, a `MidwayThreadMutex` single-instance check, thread affinity,
initialization calls, and cleanup. Candidate next calls include `00737970`, `0073d410`, and
`00737f30`; subsystem roles are not established yet.

Disk PE import descriptors name Direct3D 9 / D3DX9_40, DirectSound, DirectInput8, XInput1_3,
FMOD Ex / FMOD Event, Bink, XLive, Winsock, WinMM, and Windows system libraries. These are
dependencies to map, not implemented replacements. Raw pseudocode contains suspicious
no-return annotations on `_free`; inspect assembly/call flow before propagating them.

## Validation performed

- MSVC 19.51 Win32 Release build, `/W4 /WX /fp:strict`, Windows SDK 10.0.26100.0.
- CTest: 2/2 tests passed. Ordinary semantic fixtures cover operand order, aliasing,
  squared length, negative zero, infinity, and quiet NaN for absolute value.
- Native differential test: 455 comparisons against verified original math code, zero failures.
  Inputs are 65 integer-derived vector/scalar fixtures across seven comparisons per fixture.
- Exporter tests: 4/4 passed, covering project/path rejection, partial-export retry and
  completed-export resume, and non-PE input rejection.
- The native harness copies only the five call-free routines into its own process, supplies
  the original registers/stack arguments, and captures their outputs.
- Original game startup was not executed. No renderer, asset loader, simulation, or gameplay
  has been reconstructed or validated. There is no playable game rebuild.

Retained test output is in `reports/build_tests.txt`; raw seed evidence is in
`exports/bsp/functions/<address>/` and can be regenerated.
