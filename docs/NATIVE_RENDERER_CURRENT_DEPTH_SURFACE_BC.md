# Current renderer depth-surface field getter

The complete native B20090..B20096 body reads the current DWORD at the actual
renderer+198C into EAX and returns. Its seven bytes are
`8B 81 8C 19 00 00 C3`. Original ECX supplies the renderer; no stack arguments
are consumed. This new fastcall entry has the same physical argument and return
locations; EDX is unused. It adds no validation, pointer adjustment, reference
count change, COM acquisition or owner lifetime.

The depth-surface interpretation has a concrete producer and consumer. In full
B238D0, B23A34 calls the current device's +A0 GetDepthStencilSurface slot and
B23A5B constructs its actual surface wrapper through B3F630. After capturing the
old owner, B23A7A publishes the new wrapper at renderer+198C, with the existing
retain/release schedule. B23AE3 reloads this field and B23AEC passes it to the
full depth binder B21690. The existing native_renderer_default_surfaces source
preserves this producer; it is not a new implementation in this packet.

The installed renderer profile is D5F0A8; current slot +12C at D5F1D4 contains
B20090. In B14A10, B14DBB reloads F8D394, B14DC1 reads its current profile,
B14DC3 captures +12C and B14DC9 calls it. B14DCB then reloads the service's
current frame-target owner at +1D4 and B14DD2 calls B1FB00 with the getter result.
B1FB00 owns the actual depth-field assignment and reference-count behavior.
The getter does not return an IDirect3DSurface9 pointer or acquire ownership.

The live xref inventory has the one profile DATA reference and no direct calls.
It cannot enumerate every indirect caller. The producer and consumer paths
above justify the descriptive source name, which remains a hypothesis; the
established provisional Ghidra name is preserved for central annotation review.

The implementation follows the adjacent complete B24DC0 getter's small Win32
entry style. A strict separate source compilation and independent extraction of
its entire COFF symbol verify exact equality of all seven native bytes, with no
relocations in the code section. No runtime fixture or new repository test is
needed for this leaf. Evidence, hashes and baseline validation are in the JSON
report. The module is intentionally unregistered in this worker: its required
baseline build/tests do not include this new source. Root owns final registration,
combined-library symbol/byte verification and any native annotation.

Exact leaf code agreement is distinct from whole-renderer binary replacement,
object/provider lifetime, unrestricted invalid-memory behavior and game/runtime
validation. No claim is made about all indirect callers or alternate profiles.

Worker verification passed: strict MSVC Win32 compilation, complete seven-byte
COFF/native equality with zero code relocations, four numeric producer/consumer
call rows, current indirect dispatch bytes/profile slot, eight native seeds,
and both baseline CTests. No runtime fixture or new repository test was added.
The baseline excludes this unregistered leaf; root must check its final library.

## BC integration checkpoint

The integrator reviewed the complete native body and actual producer/provider
evidence, saved its original analysis signature and full stored range in the
existing BSP project, and registered the source where needed. Exact combined
validation follows separately from worker checks. No original binary entry,
unrestricted FH3/SEH, whole owner lifetime or gameplay claim follows.

## BC exact merged validation

Exact combined source `d6cd5f085cc8d521c31e5bf327daa10b47710e9a` passed the strict Win32 build
and both existing CTests. Camera, CString and texture probes compile only
external probe.cpp against all three current libraries; depth has a complete
seven-byte comparison in its exact library member. Coverage is bounded;
texture is hot-cache only. See `reports/native_render_service_construction_bc_validation.json`
for immutable captures, hashes, adapter scope and remaining limits. Earlier
pending statements describe worker stages. Full service/camera construction,
native ABI/FH3 compatibility and gameplay remain unproved.
