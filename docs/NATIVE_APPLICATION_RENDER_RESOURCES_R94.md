# Render-resource lifetime and application composition, R94

Addresses: 00B0F6E0, 00B14F60, 00B151C0, 00B14A10.

## Result

Three complete native lifetime bodies are reconstructed, totaling 1,629 bytes:

| Entry | Bytes | Reconstructed behavior |
|---|---:|---|
| B0F6E0 | 1305 | Release the helper's auxiliaries, visit 42 owned fields, clear the enable byte |
| B14F60 | 294 | Release members, helper/black/cockpit, vectors, marker and singleton base |
| B151C0 | 30 | Invoke the full destructor and free the allocation when flags bit0 is set |

The existing B14A10 constructor now has a persistent application provider graph
using the R92 texture cache and R93 camera domain, actual frame/default surfaces,
raw strings, original constants and a canonical CCh texture-helper companion.
It shares one F8D39C publication with BeginFrame. The raw singleton deletion map
admits D5E480 through B151C0 with these same lifetime providers. Existing host
binding offsets remain unchanged; the new pointer is appended at offset144.

The explicit `construct_render_resources()` API preserves the allocator's bytes.
Normal `GameStartupHost` startup remains gated on the later B107F0 producer and
post-effect/material integration. No new unique body credit is taken for B14A10.

## Exact lifetime contract

B0F6E0 calls B52270 on current+34 without adding a null shortcut. It then captures
+50 before capturing the CE2220 decrement import. The remaining fields are read
after each previous callback and clear. Both native visits to +1D4 remain, at
B0F912 and B0F956. Each nonnull child decrements actual+04, selects the current
zero terminal only when that decrement returns zero, then clears the current
parent field. The final +1C4 byte store occurs at B0FBEF after all calls return.

B14F60 stamps D5E480, invokes B0F6E0 and captures a fresh decrement-import epoch
for +34, +668 and +0C. It then destroys the string-header vector at+69C, the
12Ch-record vector at+68C, the captured string buffer at+684/+688 and the actual
singleton base. It preserves current vector-pointer reloads after callbacks and
the marker's captured data/current wrapping length+1 sequence.

The four-state C++ unwind projection follows DF4620/DF4640 and the original
CBC400/408/416/424/432 compiler supports. A failure runs only the remaining
member/base actions, not additional child releases. This does not establish
native FH3/SEH or unrestricted hardware-fault cleanup equivalence.

Concrete frame targets use D5E600 -> BD30E0 -> B1FCF0 and the actual surface
context. Cockpit release uses the exact constructor-owned canonical companion.
Other admitted children use the existing actual-owner registry and its current
terminal checks. No duplicate refcount or numeric host vtable is introduced.

## Startup prerequisites established from the binary

B14A10 never initializes +70. B107F0 constructs a 20h post-effect through B4E470
at B11555 and stores its result at B11563. The original application allocates
6ACh with ordinary operator_new at73DEE4 before calling B14A10 at73DF01; it does
not supply blanket zero initialization. Thus constructor completion alone does
not establish a valid lifetime for every field read by the destructor.

The first diagnostic run observed nonzero +70 heap residue (`6D617261`); the
final archived run observed zero. The former survives only as a clearly labeled
tool-transcript observation, because its runtime files were overwritten before
the final run. The original listing, rather than either allocator observation,
establishes the missing initialization. The probe explicitly admits an absent
+70 owner. Production does not add that store.

Neither B0F6E0 nor B14F60 releases noise+67C. The actual cold-load case confirms
that its reference remains1 after the complete parent destructor. The source
preserves this native omission. The probe retires that residual externally after
recording it; this diagnostic cleanup is not part of the reconstructed parent.

## Validation

- Strict MSVC Win32 build and all three existing CTests pass. No repository test
  suite is added.
- Nine native function/compiler-support spans and five data/call-site spans,
  totaling3,188 bytes, match live Ghidra and the installed PE. Thirty direct call
  rows are mechanically checked; 94 indirect rows are identified separately.
- A focused differential executes copied original B0F6E0 with its only direct
  call and import cell relocated to admitted fixture providers. The native and
  source paths match all83 decrement/terminal events across42 release visits.
  It exercises next-field replacement, a callback refilling the second1D4 visit,
  a changed import cell that must not affect the captured epoch, a nonzero count,
  callback writes overwritten by the native clear, and untouched noise+67C.
  This is an orchestration differential, not a full original parent/game run.
- The same focused application probe links51 current production application
  objects and three libraries. Full B14A10 cold-loads error.tga recursively,
  black.tga, noise.dds, kosz_01.tga, szor_01.tga, csikok.tga and splotch.tga through
  the actual VFS, D3DX, device and canonical owner graph.
- The constructor's actual frame/default surfaces, camera/cockpit, texture
  helper, marker and publication are checked. The shared service remains live
  through two ordinary application ticks/one Present.
- Direct B151C0(flags1) completes the full parent path under the explicit absent
  +70 fixture condition. Black, helper textures, frame/surfaces and cockpit/camera
  retire; publication clears. Only noise(ref1) and fallback(ref2) remain in the
  canonical render registry. After the separate diagnostic noise release, the
  ordinary renderer drain retires fallback and leaves the registry empty.
- Probe and unmodified application both exit0, worker joined, final device/API
  COM counts0/0. The unmodified application still does not call B14A10.

See `reports/native_application_render_resources_r94.json` for the complete call
rows, release schedule, native bytes,83-event trace, exact runtime limitations,
source/object hashes, saved/read-back annotations and immutable evidence archives.
The constructor's two historical duplicate ledger records are consolidated into
one current record; both complete prior records remain in the evidence report.

## Remaining work

Complete the B107F0 post-effect/material producer graph and reconcile the native
noise residual with its wider lifetime before enabling B14A10 in normal startup.
The D5E480 manager-map branch is build/static-checked; this probe directly invokes
the same B151C0 provider before ordinary manager drain. Allocation failure,
exception paths, flags0, concurrency, native ABI/FH3/SEH, pixel and gameplay
validation remain open.
