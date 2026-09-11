# Actual dynamic buffer release for device reset

`release_native_dynamic_buffers_for_reset_00b237d0` reconstructs the complete
243-byte function at `00B237D0..00B238C2` using the actual renderer and wrapper
fields, actual COM interface tables, and the existing actual synchronization
providers. Its new C++ interface takes a borrowed renderer pointer and the
borrowed eight synchronization-global bytes. The original takes the renderer
in ECX, has no stack arguments, and returns with `RET`.

The existing `d3d9_states` semantic projection is unchanged. This packet adds
actual-layout evidence without replacing or reclassifying that older evidence.
The existing descriptive Ghidra name is retained:
`BSP_D3D9Renderer_ReleaseDynamicBuffersForReset`.

## Original field and call order

| Storage | Original access |
| --- | --- |
| Renderer `+04` | Current tracked critical section, through existing actual guard providers |
| Renderer `+1974` | Vertex-wrapper pointer captured before the readiness clear |
| Renderer `+1978` | Index-wrapper pointer reread after the entire vertex block |
| Renderer `+1D8C` | Readiness byte; any nonzero value enters the release body |
| Captured wrapper `+28` | Actual COM interface pointer; independently reread after the captured pair |
| Globals `0108D6DC..0108D6E3` | Existing actual mode, observed mode, preserved bytes, and nesting counter |

When the entry-time mode is nonzero, the original first saves the renderer in
its eight-byte local guard, calls `00B33AD0`, and writes only returned AL into
the guard's first byte. The remaining three bytes of that first DWORD are not
initialized. The reconstruction uses the existing actual guard storage and
provider; it adds no initialization, lock ownership or synchronization policy.

The readiness comparison precedes exception-state arming. If readiness is
nonzero, the original captures renderer `+1974`, clears the readiness byte, and
loads the captured wrapper's `+28` COM object. A nonnull captured object receives
`AddRef` from its current table `+04`, then `Release` from its newly reread table
`+08`. The wrapper pointer itself stays captured throughout these calls.

The original then independently rereads that same wrapper's `+28`. A nonnull
current object receives its own current-table `Release`. The captured wrapper's
field is cleared only after that call returns. A null current object skips both
the call and the store. These are distinct object and table reads even when
ordinary D3D9 implementations leave the pointers unchanged.

Only after this whole vertex block does the original reread renderer `+1978`
and run the same sequence on the captured index wrapper. The C++ helper shares
the sequence while explicit DWORD loads preserve the observable rereads.
The original neither destroys wrappers nor changes pool handles, descriptors,
allocations or unrelated renderer fields.

Normal exit checks the current synchronization mode before setting native
state `-1`. When enabled, it reads the full saved guard DWORD, then the saved
renderer, and calls `00B33B00`; that provider consumes but ignores the saved
DWORD. The normal leave itself has no active cleanup state. Entry disabled
followed by exit enabled would consume uninitialized native guard storage;
that native-invalid caller domain is explicitly outside this C++ API's claim.

## Exception evidence

Fresh installed-PE and guarded Ghidra bytes establish the following complete
metadata and cleanup spans:

| Address | Evidence |
| --- | --- |
| `00CBCED8`, 10 bytes | Load `00DF54D4` into EAX, then jump to FH3 `00BF6B43` |
| `00DF54D4`, 36 bytes | FH3 magic `19930522`, one unwind state, no try/catch map, EH flags 1 |
| `00DF54CC`, 8 bytes | State 0 transitions to -1 through action `00CBCED0` |
| `00CBCED0`, 8 bytes | `LEA ECX,[EBP-14h]`, then jump to actual guard destructor `00B21110` |

Native state 0 begins at `00B23809`, after the readiness comparison and after
guard acquisition. Normal state -1 stores occur at `00B23815` or `00B2389A`,
after their respective current-mode comparisons. The C++ entry uses a cleanup
destructor armed over exactly the release body and disarmed before normal
leave. It introduces no catch/rethrow layer. The cleanup helper follows the
existing actual-provider convention for terminating a second C++ exception
during exception search. This packet does not claim arbitrary second-exception
or asynchronous SEH equivalence.

## Verification

The strict full MSVC Win32 Release build passed through `scripts/build.ps1`.
Both existing CTests passed, including native math differential testing, and
all eight installed-PE seed checks matched. The worker used an ignored CMake
registration file to include its owned source without editing shared startup.

The focused private fixture links the actual resulting `bsp_core.lib`; it does
not compile a replacement copy of the production implementation or guard
providers. Its linker map proves the release entry and helper come from
`native_dynamic_buffer_device_release.obj`, and all three actual guard
providers come from `native_renderer_synchronization_actual.obj`.

Two separate processes compare the reconstruction with complete copied original
code. All 461 bytes across twelve original entry, guard, FH3, IAT, global and
FH3-entry-prefix spans matched fresh guarded Ghidra and the installed PE.
The verifier checks every final copied byte, allowing only eighteen asserted
address relocations, the host registration binding to the copied FH3 handler,
two actual Windows critical-section IAT bindings, and a host FH3 ABI bridge.
There is no replacement buffer-release or synchronization algorithm in the
original path. The private fixture uses `/SAFESEH:NO` for copied handler code;
this is an isolated fixture build setting.

An actual D3D9 HAL device on the NVIDIA GeForce RTX 5090 creates distinct real
dynamic vertex and index buffers in `D3DPOOL_DEFAULT`. Their real descriptors,
buffer locks, writes and unlocks succeed. The fixture clones each actual
fourteen-slot table to observe reference calls. Before every observed `AddRef`
or `Release`, it restores the real object's original table and immediately
forwards that exact real Windows method. The fixture then restores observation
and performs the stated pointer mutations. It supplies no fake COM object.

Seven focused states cover readiness zero, ordinary release with captured and
current pointer/table mutations, current COM null, a captured `Release` throwing
after its real COM operation, initially null COM fields, entry mode disabled,
and current mode disabled by the final callback. The exception propagates its
original marker while the original FH3 and C++ cleanup both release the actual
Windows critical section. The mode-off exit intentionally preserves the native
held-lock/nesting state until fixture-only cleanup after observations.

The two paths match all **268,850 DWORDs / 1,075,400 bytes**, with 136 complete
renderer/wrapper/global/refcount snapshots, 24 real COM calls and 50 observed
field writes per process. Only explicit actual-pointer identity fields are
normalized. Every other renderer and wrapper word is compared exactly.
All COM call return PCs and production write PCs are verified against original
addresses or the compiled library object. Readiness clears before COM work;
the two wrapper clears occur only after their final current releases return.

All 184 runtime implementation-entry captures match the actual x86 module PE
metadata and sixteen disk bytes, allowing only normal loader relocations.
Actual buffer creation and reference methods remain in `d3d9.dll`; critical
section entries are in `ntdll.dll`; the resolved FH3 is in `vcruntime140.dll`.
The factory-only Windows `apphelp.dll` compatibility shim is allowed by the
verifier when present; this does not broaden any COM-method module check.

`reports/native_dynamic_buffer_device_release_audit.json` retains source and
library pins, every allowed original-code adaptation, compiled providers,
module provenance, exact call/write PCs, and 47 hashed private artifacts.
The private rerun helpers are `local/build_dynamic_release.ps1`,
`local/build_dynamic_release_fixture.ps1`, `local/run_dynamic_release_fixture.py`
and `local/verify_dynamic_release_fixture.py` in the worker worktree recorded
by that report. Regeneration uses `local/write_dynamic_release_audit.py`.

This is an actual-layout reconstruction with strict-build and real-resource
differential evidence. It is a new C++ interface, not a drop-in original ABI.
The installed game was read only; gameplay, rendering, concurrent mutations,
arbitrary COM exceptions and asynchronous SEH were not validated.

## Primary integration

The primary registered the source in CMake and repeated the strict Win32 build
and both existing CTests. It verified all 47 worker artifacts, four current
source/provider files and twelve fresh live-Ghidra/installed-PE spans (461 bytes).
The unchanged worker fixture linked the frozen main library, SHA-256
`2b6d41802c42d5b58a1e20702f517fc401de08d1138b4e63ebbfd0d36f9fbcaf`, and repeated the full
268,850 DWORD comparison, call/write PC checks and 184 module entry captures.
The complete reconstruction ledger and refreshed export are registered; saved
Ghidra annotation preserves the previous name and comments. No permanent test
target was added. The validation limits above still apply.
