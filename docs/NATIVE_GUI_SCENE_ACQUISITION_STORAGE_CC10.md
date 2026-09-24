# Actual GUI scene acquisition

`acquire_native_gui_scene_storage_00ac59a0` covers the complete normal body
`[00AC59A0,00AC5F5B)`: 1,467 bytes, 379 instructions and 54 CALLs (46 direct,
eight indirect). The final instruction is the one-byte RET at AC5F5A. The
source composes the published actual 24h GUI scene, 3Ch resource, 45Ch camera,
1F0h directional light and caller-owned 18Ch Group providers. Raw cells contain
actual identities. No logical GUI scene, camera or scene-resource owner is
substituted. The Group at page+4C already exists; this routine does not create it.

The explicit frame borrows 32 initialized volatile DWORDs corresponding to
F10..F8F, where F is the original stack pointer after saving EBP/EDI. Temporary
8-byte string headers, mask, allocation slots, packed color and matrix share
this one live backing. The captured EBX mask is separate from callback-mutable
F10. Provider frames are persistent initialized preimages, seeded at the native
push points. Borrowed-view providers retain their documented overlaps. The
interface excludes cross-provider private stack-address coincidence, saved
registers, return gaps and native calling-convention compatibility. It does not
replace semantically live outer values with preimages.

All contexts share one actual-count registry, current increment/decrement alias
cells, string domain, node scene/tree/world dispatch and live one cell. The
caller supplies the genuine camera and directional pools, actual current
renderer environment, reached exact virtual targets and prepared recursive
frames. Current import cells must be nonnull and callable when reached. Invalid
storage, failed/null allocations followed by native faults and asynchronous
mutation are outside the successful valid domain. No missing target becomes a
successful no-op.

## Normal schedule

1. Get the current manager and find the store using page+108. Capture the current
   page+120 gate before publishing F0. A nonzero gate and found store publish
   F4=0 and its captured scene into EC, increment that scene's actual+4 through
   the current import, then return without the later property/bounds path.
2. Publish F4=1 before allocating 24h. Construct the actual GUI scene from
   page+100 under state0; disarm before publishing page+EC.
3. Allocate a genuine camera slot under state1. Build `GuiCam_`, concatenate the
   current page name and call raw B71A80. States2/3 and bits1/2 cover the temporary
   headers. State4 disarms camera recovery before returning the concatenated
   allocation; state-1 precedes prefix return. Capture each data pointer and
   length+1 before the current string-pool getter. Prefix cleanup updates the
   captured mask without rewriting F10.
4. Get the manager again before reading current page+EC. Seed AA5070's descriptor,
   scene and captured camera cells in native push order, then publish F0. Store
   creation adds no camera/scene credits and has no insertion rollback.
5. Allocate 3Ch and construct `GuiLights` under states6/7 and bit4. Preserve the
   returned resource in F14, disarm and return the temporary. Read current
   page+EC for B723F0; decrement the captured resource using the current import,
   dispatching its current0/fresh4 only if that same actual count reached zero.
6. Reread page+EC and propagate the captured camera root through raw B6D890.
   Set flags7; zero the four scratch bytes in 2,1,0,3 order, then pass the word
   to the genuine packed-color leaf.
7. Allocate/construct the actual 34h viewport under state9. Disarm before the
   raw camera+180 setter, then decrement the captured creator/current0 on zero.
   The setter genuinely releases the camera constructor's previous viewport.
8. Allocate/construct actual 94h fog under state10 using all nine current cells.
   FLDZ/FSTP the scalar argument, disarm, set scalar68, repeat for scalar78,
   assign camera+184, then release the captured fog creator through current
   imports and the genuine current0/fresh4 family.
9. Allocate the genuine directional slot under state11. Construct its raw name
   and B7C6B0 under state12/bit8. Disarm before temporary return; normal cleanup
   deliberately leaves bit8 set. No directional creator decrement follows.
10. Capture live one, then the current directional profile and slot34 target
    before writing the 16 matrix words. Invoke the captured exact B6E870 target.
    Reread one after the callback, write direction+1E0/+1E4/+1E8 and pass the
    four diffuse words to the shared raw receiver kernel.
11. Reread page+EC for directional root propagation. Capture half, recover the
    resource from current F14, read current resource+10, write three ambient
    lanes, reread one for alpha, and invoke the shared ambient receiver kernel.
12. Read current page+108 and F0/store+18 for clear flags. After its call, reread
    the packed scratch word and current F0/store+18 separately for clear color.
13. Compare current priority and FC; publish FC before the fresh manager getter
    and AA52A0. If current page+74 is zero, capture half and FLD live D7A308,
    write sphere xyz, call the existing CRT sqrt kernel, spill/reload float32,
    call the existing CRT truncation kernel, read current page+4C, CVTSI2SS,
    write radius and call the shared raw Group bounds kernel. After the native
    pops, sphere backing is F40 and the scalar spill is F14. No float-return
    wrapper hides this interleaving.

## Source lifetime and failure boundaries

Scene, camera, resource and directional companions are separate host metadata
over the same actual+4 counts. They are admitted after the native construction
and temporary-cleanup frontiers, add no credits/native stores, and must remain
address-stable until retirement and callback quiescence. Scene admission follows
the native EC publication. The actual directional constructor establishes the
node/atomic prefix; after its temporary cleanup, a byte-preserving host operation
starts the 12-byte `SystemAmbientBacklinks` lifetime at +178. It creates no
logical tail. Admission failure retains that prepared descriptor and completed
payload for explicit genuine Light cleanup.

These outer host-admission failures bypass native cleanup and preserve the exact
native state and completed credits. The existing resource provider has a distinct
nested ambient-admission failure: before propagating, it already destroys its
registry/name/base prefix, while retaining the completed raw ambient creator.
The outer routine records that consumed prefix and skips additional AC59A0
cleanup. State7, the live GuiLights header/mask, raw 3Ch allocation and ambient
credit remain available. The nested reported state2 is historical, not a pending
second cleanup. Explicit disposition must account for the already-destroyed
prefix; it must not rerun the resource destructor on it.

Native construction failures consume each outer state before its cleanup. They
return only the current protected raw allocation or masked temporary string;
they do not destroy completed published scenes, cameras, resources, viewports,
fog, lights or store allocations. Persistent acquisitions retain those residual
credits/publications and provider diagnostics. A second cleanup exception
terminates. This is a source C++ projection, not native FH3/SEH transport.

The DEF8D4 descriptor and DEF8F8 map contain all 14 states:

| State | Previous | Handler | Protected value |
|---:|---:|---|---|
| 0 | -1 | CB88E0 | Current F14 allocation |
| 1 | -1 | CB88EE | Current F14 camera slot |
| 2 | 1 | CB88F9 | Bit1, header F18 |
| 3 | 2 | CB891B | Bit2, header F30 |
| 4 | -1 | CB88F9 | Bit1, header F18 |
| 5 | 4 | CB891B | Bit2, header F30 |
| 6 | -1 | CB893A | Current F14 allocation |
| 7 | 6 | CB8948 | Bit4, header F28 |
| 8 | -1 | CB8948 | Bit4, header F28 |
| 9 | -1 | CB8967 | Current F10 allocation |
| 10 | -1 | CB8975 | Current F10 allocation |
| 11 | -1 | CB8983 | Current F18 directional slot |
| 12 | 11 | CB898E | Bit8, header F30 |
| 13 | -1 | CB898E | Bit8, header F30 |

The ten cleanup functions plus CB89AD handler occupy 215 bytes. The captured
221-byte cluster also contains six trailing INT3 bytes, which are not claimed
as function code. Ghidra was read-only. Boundary listings with truncated returning
free tails and the undefined CB89AD handler are reported for primary repair.

## Evidence

Fresh live Ghidra bytes equal the installed PE for the complete body, compiler
cluster, descriptor/map and borrowed profiles/constants. The original frozen
readiness archive remains unchanged; its phrase “46 calls” describes the direct
set. The new report enumerates all 54 CALLs and the exact cleanup transfer sites.

Strict MSVC Win32 compilation and both configured existing CTests passed. One
ignored copied-original/source comparison runs full new acquisition followed by
store reuse, with the real production renderer/HAL/camera pool and genuine raw
graph providers. It compares the declared scalar fields, count-call sequence,
signed-zero ambient/center inputs and x87 radius outputs. Separate assertions
check actual identities, canonical +4 bindings, root/head/successor links,
resource relationships and complete explicit retirement. The raw registry
pointers at resource+1C/+28/+2C/+30 are excluded; this is not whole-object byte
equality. Registry count/mask/active fields remain in the scalar comparison.

The original copy replaces five bytes at AC5D85..AC5D89 with a bridge that
executes the current profile/slot34 loads at the capture point and supplies the
genuine B6E870 adapter. Those original instructions are not independently
compared. Forty-six direct calls use genuine source providers; nine import or
constant operands are relocated. Original-side companion admission/preparation
occurs in successful constructor bridges before outer temporary cleanup; no
admission-failure equivalence is claimed. Three outer observed-zero virtual0
arms, null-allocation branches, cleanup paths and callback-mutated inputs are
not dynamically exercised. Existing provider checks support those boundaries.

The final application run exits zero, completes ordinary drain and releases the
renderer device/API COM references to zero. It uses fixture-initialized type
descriptors/guards with genuine same-token bootstrap predicates, isolated genuine
weak/directional/Group pools, and a constructed 88h GUI manager. The weak lifetime
has the exact mutex destructor. The actual registry, renderer/HAL, camera pool
and VFS strings come from the production application fixture. Graph companions
share that registry; no later-startup admission is claimed. The report preserves all eight
attempt logs and distinguishes fixture assertion/projection mistakes and an
invalid mixed-header/library build from source/native mismatches. Final input,
object, core-library, response-file and executable hashes are pinned together.

The focused fixture's full domain and limits are recorded in the accompanying
report and retained local evidence directory.
No tracked test, automatic application admission, binary replacement or gameplay
claim is added. Existing logical APIs remain separate.
