# Native resource manager in application startup

## Result

Phase 6 now calls the actual reconstructed resource-manager and parser bodies.
`GameNativeResourceApplication` owns the `010901C4` publication and the eight
parser publication cells. Its contexts borrow the application's existing
`01090AA0` singleton manager and raw `01090AA8` string pool. The previous
`GameResourceManager` and `GameStructuredParser` semantic registry wrappers have
been removed from the application.

`004C1400` constructs the real `28h` manager through `B81040`, including its
default factory and both native sentinel trees. That constructor registers
Mesh, SkinedMesh, SkinedMeshAnimation, MatrixIndexedMesh, Camera and GroupParams.
Phase 6 then gets the actual `8h` AnimationChannels and Bone singletons and
registers them through `B80A50`. Diagnostics read the actual parser-tree count at
manager `+10h`; the completed startup contains eight parser entries.

## Call and ownership evidence

All forty original bytes at `0073DB41..0073DB68` match the installed PE and live
Ghidra. The phase-6 schedule is preserved:

| Call site | Native target |
| --- | --- |
| `0073DB41` | `004C1400` resource manager |
| `0073DB48` | `00736DD0` AnimationChannels parser |
| `0073DB50` | `00B80A50` register parser |
| `0073DB55` | `004C1400` resource manager again |
| `0073DB5C` | `00736EA0` Bone parser |
| `0073DB64` | `00B80A50` register parser |

The application installs the manager's existing `D63128`/six-parser deletion
context and the extra `CFEA34`/`CFEA44` secondary contexts before any getter can
register an owner. The shared `BD0400` drain dispatches the two actual secondary
thunks, including their `-4` adjustment. No additional owner count, manager,
container, string pool or native allocation policy is introduced.

Raw string services can be borrowed before VFS core startup. Their existing
deletion bindings are now installed when `GameNativeVfsApplication` finishes
construction, so an early resource operation can create a string pool that the
same singleton drain can destroy. Core startup still performs its original
type, provider and physical-pool initialization separately.

Interrupted resource operations record their entry address and reject replay.
Phase 6 uses the existing application retention guard; uncertain native cleanup
does not discard its live contexts. The validation callback invokes the current
source CRT `_invalid_parameter_noinfo`, including a returning installed handler.

`GameStartupHost` drains native owners while all contexts are alive, then deletes
its singleton host before its VFS host. Resource-context destruction therefore
does not access borrowed publication or deletion-table references. It performs
no native destruction a second time.

## Validation

The strict Win32 build and all three existing CTests pass. A focused ignored
probe links the built `bsp_core.lib` and the forty-four built application object
files, excluding `game_main.obj`. The controlled child bootstrap maps verified
original read-only data; it does not execute original image code.

Two children exercise the production `GameVfsHost::phase6`: one before VFS core
startup to check the exposed raw-service lifetime, and one after actual phase 2.
The latter performs three loose mounts and two package scans in the worktree.
Both check all eight names through the native tree and all eight parser profiles,
shared string/lifetime identity, duplicate registration returning false, stable
manager identity and complete shared destruction. They drain ten and fifteen
registered owners respectively. Every parser, resource-manager and string-pool
publication is cleared. Destruction then follows the application's order:
singleton host first, VFS metadata afterward. Both children exit successfully.

No permanent tests were added. The report retains call rows, original-byte and
probe/artifact hashes. Hardware-profile comparison was unavailable in the second
child because the existing HKLM profile was incomplete; that probe did not write
the profile, launch a game window or render a frame.

## Remaining boundaries

This installs manager construction, registration and lifetime. It does not
install the entire native resource-load/cache/parse dispatch graph, execute the
AnimationChannels/Bone item parsers, or supply texture/render owners. Native
register ABI, original FH3/SEH and CRT exception identity, allocation-failure
recovery, concurrency and gameplay remain unvalidated.
