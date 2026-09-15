# Constructed renderer and installed effect admission

The complete reconstructed `B32410` renderer constructor now executes before
installed `debugshader.shfx` admission. Its actual outputs replace the earlier
fixture's renderer profiles, capability cells, D3D9 factory, shader-state
definitions and system-constant initialization. The same VFS, raw singleton
manager, string pool and canonical resource registry serve the combined path.

This extends [cold effect admission](NATIVE_INSTALLED_COLD_EFFECT.md). Production
sources and root libraries are unchanged. Earlier constructor evidence proved
compilation and linking without invoking the parent; this capture executes its
complete successful source path and then consumes the constructed renderer.

## Observed behavior

`B32410` publishes the actual renderer and its primary/secondary profiles,
constructs its render cache and embedded registries, creates the real D3D9
factory, renderer Lua owner, state definitions and system constants, enumerates
display resolutions and runs complete `B2C8E0` capability gathering. Its embedded
worker locks, control worker and final critical section are also created.

An independent real `GetDeviceCaps(0, HAL)` query agrees with the native
published maximum texture dimensions and pixel-shader version. This run reports
16,384 x 16,384 textures, shader versions 3.0/3.0 and native ATOC flag one. These
are hardware observations, not portable expected capabilities. The fixture
creates its 64 x 64 windowed HAL device from the constructor's actual factory.

Cold loading still resolves `error.tga` and `white.tga` to the installed DDS
files. Their 22,000 and 11,064 retained bytes match exactly. Both are 128 x 128
with eight mips, DXT5/DXT1 respectively. Error/white native counts remain 3/5;
two backing allocations and cache byte accounting remain 33,064. Their actual
stream/backing reference counts and ordered resolved/request aliases pass.

The effect produces primary modes 0, 1, 3 and 4, secondary mode 0 and one
retained primary. All eight actual HAL shader bytecodes equal installed cache
rows 156 through 163. Cursor 164, canonical owner count 28, state caches 2/1/1,
shared state/pass/shader identities and reference counts remain correct. VFS
counters report 14 reads and 2,301,268 bytes.

The constructor creates a real dormant control thread and two real auto-reset
events. Its native initial `run=0` state is preserved. After effect validation,
actual `B33B50` signals shutdown, joins the thread, closes its handle and
destroys both events. A duplicated OS thread handle independently confirms
termination with exit code zero. This retires that child; remaining renderer,
Lua, resource and manager owners remain allocated through process exit.

## Evidence

The successful result is
`local/native_constructed_renderer_effect/fixture_result_1.json`; the source is
`renderer_effect_probe.cpp` in the same directory. The first compile stopped on
an alignment-padding warning in the fixture. Naturally aligned DWORD scratch
arrays preserve the required raw scratch extents and remove that warning.
The revised fixture and all 24 explicit dependency units pass strict MSVC Win32
`/O2 /Oy- /W4 /WX /fp:strict`. The final link uses 23 explicit units and verified
root-library providers; no new CMake project, source registration or test suite
was added. The previous closed fixture was copied without modification.

Live Ghidra and the pinned installed executable agree on all 5,350 bytes of
`B32410` and `B2C8E0`. All 42 selected direct call sites pass the live
instruction/body checker. The original `Direct3DCreate9` import thunk and 64
bytes of original constant/event/clock/control profile data were also captured.
Copied profile inputs and the original empty sampler body were rechecked
against that executable. The report retains the source and runtime boundaries,
physical Win32 modules, installed-input hashes and immutable build provenance.

`B32410` originally takes the fresh renderer in ECX, returns it in EAX and uses
plain `RET`. The source interface adds borrowed context in EDX. `B2C8E0`
originally takes ECX and uses `RET`; its new source interface likewise receives
explicit scratch/lifetime context. Original full constructor/gather machine
code is not executed. Only the previously captured `B3B280` empty branch runs
as original code; full paths run reconstructed source.

## Remaining boundary

The fixture supplies aligned raw renderer storage, the required live
`CameraPlaneSet` lifetime with restored entry preimages, zero-initialized raw
mode/identifier/capability scratch, PC/USA settings and one installed loose-file
mount. Original ignored-HRESULT and unwritten-output behavior is retained; no
failure output is synthesized. The successful run does not test late parent
failure or cleanup during unwind.

HAL device setup and material-policy cells remain fixture inputs. The control
thread stays dormant: clock sampling and BeginFrame/EndFrame providers are
explicitly absent and are never reached. The declaration registry stays empty;
real empty-array cleanup is bound, while decode-only providers are absent.
This does not establish device startup through `B2AEB0`, a running render loop,
declaration loading, complete application/archive startup or shutdown, native
FH3/SEH/replacement ABI, drawing or gameplay. Installed variant-cache writes
remain disabled, and installed assets are hash-checked before and after the run.

The immutable capture is `local/checkpoints/aa2f6e7b/constructed-renderer-effect/validation.json`,
SHA-256 `937bac2f621c866e3407a41d35c28faf568c1d4c90ee5ca6a91af58e2473e1a3`. It freezes 1298 artifacts,
80 physical Win32 runtime modules and 291 root-library provider sources. The
fixture directory is closed and read-only; copy it before further experiments.
