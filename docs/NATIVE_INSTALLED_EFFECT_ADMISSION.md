# Installed effect admission through actual owners

The actual `B2EBB0` loader now has a passing installed-data admission fixture.
Starting with `debugshader.shfx`, it resolves the basename, constructs the
actual effect with `B407A0`, runs `B46950/B45EE0`, assigns the original name,
and registers the resulting canonical effect owner. Its four primary passes
and one secondary pass are created by the reconstructed children. All eight
primary shader bytecodes match the installed cache through Direct3D9 HAL
`GetFunction`.

This advances the previous installed compiler fixture: the real loader now
produces descriptor paths, program names, mode requests and owned pass slots.
It uses one actual VFS manager, string pool, singleton lifetime domain and
canonical render-owner registry throughout.

## Composition and ownership

The source physical-provider pool, `BEDA60` manager/factory constructor and
`BE1890` mount create the loose-file VFS. The committed VFS search sources
from `orch4-vfs-application-bj` at `2cccac2e` execute all 78 ordered
`00738360` registrations using actual group/list/tree producers. The loader's
`BDF4C0` calls resolve the root descriptor and four `dummy.shfx` mode names;
no fixture callback substitutes a successful filename.

`B3A600/B38A70/B35340` acquires the 1,628-record installed cache. The root and
four mode descriptors run through actual Lua bootstrap, fundamentals,
`DoFile`, override handling and descriptor readers. A fresh VFS has no Lua
override suffixes, so the real override producer follows its empty-list path.
The final VFS accounting is 12 reads and 2,268,204 bytes: the cache,
fundamentals, one root descriptor, four mode descriptors and five includes.

The loader generates keys for modes 0, 1, 3 and 4 with policy F/generation 3.
The compiler strips the directory and finds rows 156–163; cursor ends at 164.
Every other primary slot is null. The real secondary producer constructs
slot 0, copies the primary shader/state owners, applies its state changes
and clears the remaining secondary slots. Normal descriptor cleanup,
pass finalization and primary-0 retention complete before canonical admission.

The checks establish:

- Actual effect identity `D61A00`, reference count 1, serial 0 with the shared
  serial incremented to 1, priority 23, pipe ID 0, original name preserved,
  retained root descriptor and no shadow pass.
- Four primary passes, one secondary pass, primary-0 retained in the base
  owner's array. Primary-0 has two references; secondary-0 has one.
- State caches contain two render owners, one third-state owner and one
  sampler-state owner. The primary render owner has five references, the
  secondary render owner two, and the shared third/sampler owners six each.
- Primary modes share canonical state identities. The secondary shares the
  third/sampler owners and both shader owners with primary-0.
- The actual hot white texture has six references, the error texture two,
  and the completed registry contains 28 canonical owners. The admitted
  effect resolves to its loader-created companion.

| Installed program | Bytes | SHA-256 |
| --- | ---: | --- |
| debugshader0F3.vso, 1F3.vso, 3F3.vso, 4F3.vso | 432 each | cbc4d11e94dfd24513e4d433bebb1c6a4926f47146509c6643ea32ca027d586a |
| debugshader0F3.pso | 444 | 1010de3ba58511098c5d1800239975d09175cf3b1ceb010b82688b388840033a |
| debugshader1F3.pso, 4F3.pso | 200 each | 3751a656ab27ea7c382e92625f76615abcbbbfca409f7d4c453f60b6c8686ae0 |
| debugshader3F3.pso | 348 | 909be267fb068b24571d8dc13d45796515d274d82d95dbaa57047adcd1c46205 |

## Build and evidence boundaries

The probe compiles 24 source units plus the harness with MSVC Win32
`/O2 /Oy- /W4 /WX /fp:strict` and embeds its manifest. Eighteen root units
whose include graph reaches the extended program context are rebuilt against
the same header. The two cache units, reviewed secondary implementation,
and three committed VFS search units are compiled explicitly. Source copies
remain byte-identical to their recorded origins. The unchanged root libraries
are verified against their existing immutable source/build checkpoint.
Pure source-compilation providers can be removed by `OPT:REF`; the report
distinguishes compiled units from linked units.

The first two compilation attempts caught harness variable collisions and a
missing include search directory for existing source literals. Both runtime
captures pass; the second adds the state-cache identity/reference assertions.
The existing controlled-child bootstrap reserves the five input-data bands
before the initial thread. It releases only its own verified untouched
reservations before the unchanged read-only mapper runs.

Fresh direct call-site bytes, function spans and profile slots are compared
with live Ghidra and the pinned original PE. The report separates 90 direct
calls from two indirect pass-finalizer calls resolved through `D61BE8+0C`.
The original 306-byte `B3B280` body executes twice only on its empty branch
to observe unconsumed entry preimages; all full admission execution is source.
Earlier copied dispatch profiles are rechecked against the same installed PE.

Existing names for `B2EBB0` and `B46950` are retained. Their evidence is
extended in the name shards and saved Ghidra comments under the write lock;
prior comments are retained, old values logged, readback checked and exports
refreshed. Descriptive names remain reconstruction hypotheses.

Original ABIs remain distinct from the explicit source APIs: `B2EBB0` has
unused registry ECX, two stack arguments, EAX owner and `RET 8`; `B46950`
has ECX effect, stack filename, AL result and `RET 4`; `B45EE0` has ECX
effect, three stack arguments, AL result and `RET 0Ch`. This fixture proves
the successful flag-0 source path, not original FH3/SEH or binary replacement.

Hot error/white textures, raw renderer/capability/material-policy cells and
PC/USA inputs remain explicit fixture inputs. It does not cover cold texture
loading, full renderer/application construction, archive startup, variants
writes, complete teardown, drawing or gameplay. No whole-cache destructor
is supplied; the successful process retains its owners through exit. Every
installed input remains hash-identical before and after each run.

Root C++ and CMake files are unchanged. Separately leased cache/secondary
sources and committed VFS sources were consumed as review copies. The report
pins private commands, failed build captures, both successful runs, complete
compiler dependencies, actual binaries and physical runtime modules.

## Follow-up packets

Bring the reviewed dependencies through the owning integrator's source
registration once their leases permit, then replace the remaining renderer,
texture and policy inputs with application-owned producers. Use the admitted
effect in the actual submission path before claiming drawing or gameplay.

Immutable checkpoint: `local/checkpoints/25649783/installed-effect-admission/validation.json`,
SHA-256 `89038d35de1874772202b4b31e6f5ace2d40c09f09b064b4f9a6e4316e8583e5`. It freezes 1,373 artifacts,
80 physical Win32 modules and 213 root provider sources matching the existing
library closure. None of the 18 extended-context units was taken from the old
root library. The compiled-only unit is `native_render_service_texture_construction`.
