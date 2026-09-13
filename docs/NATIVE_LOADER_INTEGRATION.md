# Native shader and VFS integration checkpoint

Addresses: 00B43B00, 00B69D40, 00BDF4C0, 00BDDC80, 00BDEB40, 00B26500, 00B265C0, 00B26680, 00B22650, 00B226B0, 00B22710, 00B5EA10, 00B5EAA0, 00B5EB40, 00B3F630

The isolated `agent/orch5-20260911` worktree combines the actual shader
descriptor reader, VFS name resolver and state caches, with the reviewed MPAK
and device-route prerequisites. Surface ownership can use the same actual
string pool. A concrete five-child adapter connects the descriptor reader,
name resolver and state caches to the existing material-program interface;
the compiler remains a required dependency.

Exact validated code: `c5e1427f80bef38c28277c22c437345856e37d52`. MSVC Win32 Release with
warnings as errors and both existing CTests passed. The combined target used
a temporary additive source include under `local/`: another worker's active
lease prevented default registration of the three listed sources. This is
an isolated worktree checkpoint; it has not been promoted to main.

One original/source fixture passes for all nine state-cache bodies, including
directional duplicate matching, reference transfers and distinct statistics.
A second fixture resolves the installed shader through the actual physical
VFS route, executes the gated native SRCH builder, then parses two installed
shader files through the same real Lua/VFS/string domains and performs
canonical shutdown. The fixture's manager/mount records are explicit inputs.
It does not exercise all alternate VFS candidate passes. Cache terminal
reentry and populated growth remain assembly-reviewed rather than fixture-tested.

All 463 direct call rows across the six component/dependency reports pass.
Indirect calls retain separate evidence. Nineteen annotations were saved and
read back with prior values journaled, and their exports refreshed. Three
state-vector post-free flow gaps were repaired. The Lua B69D40 body now reaches
its real B69DE4 end: the post-free tail was decoded, the truncated function
recreated with its existing name/comment preserved, and all 55 instructions
verified with zero remaining gaps.

## Follow-up packets

Append shader-reader, VFS-resolver and effect-runtime source registrations when
the shared registry lease clears; run the default build and fixtures at the
exact commit before promotion. The texture branch remains outside this
checkpoint while its author fixes retained resolver header lifetime and
publishes explicit per-call child state. The separate compiler worker is
recovering the actual builder/wrapper and field prefix, with the native
compiler tail still explicit. Complete cold effect loading, GPU texture
loading, original binary ABI and gameplay are not validated.

`reports/native_loader_integration.json` pins source, library-validation recipes,
logs, saved-analysis journals and archived worker evidence. The permanent
test suite was not expanded.
