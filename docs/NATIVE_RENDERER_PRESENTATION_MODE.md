# Actual renderer presentation-mode change

Addresses: 00B29E60

| Routine | Native range, inclusive | Coverage | Source interface |
|---|---|---|---|
| `BSP_D3D9Renderer_ChangePresentationMode` | `00B29E60..00B2A063`, 516 bytes | Complete normal body and optional-guard cleanup behavior over the admitted actual-storage domain | `change_native_renderer_presentation_mode_00b29e60` |

The descriptive name is a hypothesis, not a recovered symbol. The native ABI is
`__thiscall`, ECX actual renderer, six DWORD stack slots, and `RET 18h` at both
`B29F3E` and `B2A061`. Only AL=1 is semantic; the routine does not establish
EAX=1. The slots represent width, height, fullscreen low byte, multisample DWORD,
interval-selector low byte and force low byte. The new C++ interface adds a
borrowed context and retains all six raw DWORD slots.

Settings call `8D6092` and main-menu call `583087` each push six DWORD arguments.
The settings producer reads dimensions at settings+14/+18, fullscreen at +1E,
multisample at +58 and interval selector at +60. The menu producer reads the
corresponding current F88994/F88998/F8899E/F889D8/F889E0 globals and forces an
update. These callsites agree with the full callee listing and both RET18 exits.

## Actual domain and producers

`NativeRendererPresentationModeContext` borrows the completed AW recreation
context and the actual pending byte `0108D4B8`. Synchronization, current D5F0A8
profile and gamma use the same references as recreation. No renderer,
presentation structure, lock, pending flag or resource domain is mirrored.
The actual renderer must remain valid through +1D93, together with every reached
AW dependency. Profile +2C is B1FE20 and +F0 is B21960; both current table words
are read before invoking their concrete full providers.

Native initialization `B2AEB0` clears 38h bytes at renderer+1A28 (`B2AEDC`) and
writes the D3D presentation members before real CreateDevice. It writes width at
`B2AF52`, height at `B2AF78`, Windowed at `B2AF8E`, and publishes renderer+1A20/+1A24
at `B2AF9E/B2AFA4`. Constructor B32410 initially clears those latter fields at
`B32528/B3252E`. It stores the actual BD1860 tracked-lock result into +199C at
`B328DF`, clears pending at `B327B7`, and clears inhibit+1D90 at `B3278C`.
The current gamma provider B21960 publishes its cached value at +196C.
Full startup, settings, menu and presentation owners remain separate integration
work; these producer observations do not claim their reconstruction.

## Behavior and call contracts

| Native site | Consumed provider | Established behavior |
|---|---|---|
| B29E8B | Full B33AD0 | Current optional mode gates entry; save renderer then entered AL. |
| B29EA3 | Real EnterCriticalSection | Capture lifecycle+199C before arming state0; increment that captured lock+18 after entering. |
| B29F09 / B2A02C | Real LeaveCriticalSection | Both normal exits reread current lifecycle+199C, decrement its actual DWORD depth and leave it. |
| B29F25 / B2A048 | Full B33B00 | Read current optional mode before disarming cleanup; pass the full saved local word if enabled. |
| B29FB5 | Full actual B29670 | Changed raw Windowed DWORD invokes the complete AW recreation over the same actual domains. |
| B29FEE | Current D5F0A8+F0 / full B21960 | Publish current dimensions and call gamma with the actual outgoing x87 float spill. |
| B29FF7 | Current D5F0A8+2C / full B1FE20 | Read actual frame-active DWORD+1998; only AL is the result. |
| B2A01D | Current device COM+A4 | Inactive frame, inhibit0 and lost0 call real BeginScene; ignore HRESULT. |

The interval-zero test is read unconditionally before the force test. The early
match compares normalized stored Windowed against the **raw fullscreen byte**
using inequality, and compares the interval-zero result against the **raw
interval byte** using equality. Normalizing input slots to `bool` would change
the noncanonical-byte cases. Nonzero upper bytes of flag slots are ignored.

On update, capture whether old raw Windowed differs from `fullscreen_byte==0`.
Nonzero dimensions overwrite their presentation fields; zero dimensions retain
current values. Preserve stores in this order: inhibit+1D90=2, swap effect+1A40=1,
multisample+1A38, quality+1A3C=0, interval+1A5C, Windowed+1A48, flags+1A54=2.
The native NEG AL / SBB EAX / AND / ADD interval computation yields 80000000 for
a zero low byte and zero otherwise. Equal Windowed writes pending=1; changed
Windowed invokes AW immediately and leaves pending untouched by this parent.

After AW, native reads presentation width, executes FLD gamma, reads height and
captures the current renderer profile. It publishes width+1A20, executes FSTP
to the outgoing float word, publishes height+1A24, then reads profile+F0. The
small assembly helper preserves that sequence and hands the same outgoing word
by reference to the existing full gamma provider. It then reads a fresh current
profile for the frame-active call. No fixed numeric native call is used in
production; concrete reconstructed providers and current real COM calls execute.

## Exception evidence and limits

Raw handler `CBD348..CBD351` is ten bytes and currently has no standalone saved
Ghidra function. It loads FuncInfo `DF5B28..DF5B4B` and jumps to BF6B43. Its single
unwind entry `DF5B20..DF5B27` is `(-1, CBD340)`. Existing action
`CBD340..CBD347` forms the local guard address and tail-jumps to B21110.
These are read-only evidence dependencies, not additional reconstructed bodies.

Only the optional guard has EH cleanup. A failure during nested AW recreation
leaves both acquired lifecycle entries and all completed writes intact. The
source intentionally does not add lock rollback. Skipped-entry guard bytes are
uninitialized; later enabling their cleanup is outside the valid caller domain.
The source preserves the observed cleanup behavior but does not claim native
caller stack identity, unrestricted FH3/hardware-SEH equivalence, arbitrary
profile domains, concurrent worker/device safety or gameplay integration.

## Verification and durable replay

Live Ghidra and installed PE bytes match for the 516-byte body, both raw EH
spans, B1FE20 and the admitted renderer profile. Analysis used
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`; no Ghidra mutation was
performed. `reports/native_renderer_presentation_mode.json` records hashes,
native prototype, callsites, build results and fixture inputs.

Strict MSVC Win32 `/W4 /WX /O2 /MD /fp:strict` compilation passed. The standard
build passed both `reconstructed_math` and `native_math_differential` after
`verify-seeds`. Worker ownership excludes shared build registration; the strict
compile and explicit worker-source fixture compile the new translation unit.

The isolated original-byte fixture at
`C:/Users/sqz269/bsp-ax-presentation-mode` passed nine comparisons: 73,440
normalized bytes and 3,210 event words. It executes the complete original mode
body with five documented operand relocations, original AW body with its
thirteen relocations, original FH3 handlers/maps, and current concrete providers.
It compares AL and native RET18/callee-saved registers, raw flag early returns,
zero dimensions, interval selection, seven incoming x87 values, pending updates,
current output dimensions, current lifecycle replacement and nested exception
cleanup. Real RTX 5090 HAL devices, real retained 2D/cube/volume inputs, one
hardware-layout tree node, dynamic VB/IB and real BeginScene execute.

Changed Windowed tests use stored raw value2 followed by normalized value1, so
both real devices remain windowed. Output mutation, inhibit/frame-active
changes, current lock replacement and a C++ throw are explicitly instrumented
branches. The private window stays hidden and the foreground window is checked
unchanged. Gamma is finite/equal or disabled in the seven-value x87 case; these
results do not prove arbitrary gamma exceptions. Online is zero and logical/
shader arrays are empty, as in the borrowed AW fixture domain.

Initial replay uses the explicit `-WorkerSource` switch plus the pinned
`fbab26c3` three-library snapshot. After integration, `run.ps1 -Root <checkout>
-LibraryRoot <checkout>` defaults to compiling only `probe.cpp` and linking the
three supplied current libraries. No worker object is silently included.

## AX integration analysis refresh

The integrator saved all four AX original signatures and reviewed names,
verified their complete stored bodies and refreshed exports. Three ten-byte
EH handlers CBCC0E, CBD348 and CBD408 were defined under owned leases and
the Ghidra write lock. Missing-function observations above describe the
earlier worker capture. EH definitions are analysis metadata, not additional
reconstructed normal-body claims. Combined final-commit validation remains
separate from the worker fixture evidence.
