# Native compiler and texture integration checkpoint

Addresses: 00B43B00, 00BDF4C0, 00B26500, 00B265C0, 00B26680, 00B3C3A0, 00B354D0, 00B35930, 00B3A7E0, 00B3B3C0, 00B319B0, 00B30B40, 00B31BD0, 00B31C20, 00B2C2D0

The isolated `agent/orch5-20260911` branch combines the actual shader reader,
VFS resolver and state caches with the recovered compiler builder/wrapper
prefix and corrected texture-loading cache. The effect adapter publishes the
actual compiler operation before work. Texture binding creates a distinct,
persistent actual VFS operation for each invocation and checks both the actual
string pool and current-manager publication slot. The cache fix keeps name
headers alive through failed child teardown; consumed headers are preimages.

Exact validated code: `8bcdfac3299c82a89eb670b576091c17f38f4ba0`. MSVC Win32 Release with
warnings as errors and both existing CTests passed. Five focused fixtures
link against the combined `bsp_core`: original compiler builder/lifetime,
original state caches, actual installed shader/VFS parsing, texture record
growth, and deliberately injected resolver failure with persistent headers.
All 788 direct and four resolved indirect call rows passed verification.
Remaining indirect calls retain separate evidence.

The build uses a temporary additive source include under `local/` because
another orchestrator's lease still covers `cmake/startup.cmake`. Five source
registrations and promotion to main remain pending. This checkpoint does not
claim a default registered build or a runnable reconstructed game.

Eighteen new function annotations were saved, read back and exported. Seven
post-free gaps in B3A7E0 were repaired, then its full body was recreated through
the B3AE9B return: 618 instructions, zero gaps. The 23 formerly unowned tail
calls now pass verification. Five compiler comments also record this repair
while preserving prior evidence. Compiler and texture worker artifacts were
archived with hashes; their source-only texture import remains explicit.

## Follow-up packets

Register the five sources after the external lease clears, then validate the
exact merged commit before promotion. Complete the required compiler tail at
B3B513/B3B536 using the actual D61810 reflection owner under review. Integrate
the separately recovered named cube/volume constructors into the real texture
loader and canonical ownership path. Retry/nonzero callbacks and renderer
registry teardown remain explicit dependencies.

The new adapters have independent source review; their complete runtime
composition is not fixture-tested. The resolver fault case uses a diagnostic
throwing provider, not successful GPU/cache execution. Full cold effect load,
original binary ABI and gameplay remain unvalidated. Detailed source, artifact
and validation pins are in `reports/native_loader_integration.json`.
