# Typed particle model publication views

The construction and lifetime access views now borrow the existing typed
`NativeParticleModelManagerStorage* const volatile&` publication cell. This
removes the incompatible `void*` reference type without changing either function
body or introducing a publication mirror, manager owner, or model activation.

## Accepted baseline and exact change

This packet starts from accepted early-manager-startup main
`a51a734bb75de73bb5c464e60a1e758192838585`, after the typed-view readiness
`d0ff29d9328da79564751e72c4877175b857cec2` was integrated. The actual C++ commit is
`99b170231e937da304703fe01221bcc8b8b8aef7` and changes exactly two headers:

- `native_particle_model_construction.hpp`: forward-declare manager storage in
  `namespace bsp` and change `manager_00f8c274` to the typed reference above.
- `native_particle_model_lifetime.hpp`: change its `manager_00f8c274` field to
  that same typed reference. Its existing construction-header include supplies
  the forward declaration.

Both references bind directly to the existing typed volatile publication cell.
The added `const` qualifies the pointer object through the borrowing reference;
the manager pointee remains mutable. Binding performs no pointer-value read,
temporary-pointer construction, reference cast, or ownership acquisition.

The unchanged constructor passes the current pointer value to AF0950, whose
existing `void*` argument receives the normal pointee conversion. The unchanged
destructor captures the current manager value once, after current variant and
mesh release callbacks, then decrements the live count and calls AF0AE0 with
that captured value. Its local capture is the existing per-call value, not a
new publication cell or a value cached in either access structure.

The permanent publication cell does not extend the manager pointee's lifetime.
Existing model access objects must outlive their borrowers through terminal and
failure cleanup; models must unregister before the manager's weak-array drain.
This change adds no concrete access owner, lifetime adapter, retain, lock, or
missing callee implementation. The accepted baseline already has the separate
early-manager-startup change; this packet adds no further activation.

## Captured compiler and Source evidence

Before copying inputs, the capture qualified every one of the accepted Root
startup epoch's 3,352 inputs and four build artifacts against complete frozen
bytes and the live Root build. All 3,356 Root pins matched exactly. The worker's
seven newline variants are retained as separate whole byte preimages; their
LF-normalized contents agree. Both changed headers have exact baseline bytes.

All 56 retained `CL.read`, `CL.write`, `CL.command`, and `Cl.items` tracking logs
across the 14 Release target directories were searched. The actual compiler
dependency cohort for these two headers contains seven Core translation units:

- `native_particle_array_resize.cpp`
- `native_particle_cone_definition.cpp`
- `native_particle_model_construction.cpp`
- `native_particle_model_lifetime.cpp`
- `native_particle_model_update.cpp`
- `native_particle_record_children.cpp`
- `native_particle_record_update.cpp`

Their complete read groups, command records, items rows, write groups, resulting
objects, and matching whole Core archive members are retained for both phases.
The selected Source set is the union of the accepted 3,352-input epoch with this
actual dependency set and build inputs: 3,358 whole inputs per phase. Every
selected path has complete committed Git bytes plus complete worker/build byte
variants, not merely a hash label. This is a selected consumer read-set proof,
not a claim to cover every input of all 14 targets or all App object bodies.
External compiler/SDK dependencies are recorded as paths only; their whole
header/toolchain preimages are outside this packet's evidence boundary.

The captured actual commit objects and directory trees independently bind each
selected blob to the before and after commits. A separate live-Git verifier
also checked all 3,358 committed blobs in each phase, all 3,358 current worker
files, and the exact two-file C++ commit delta. The seven newline differences
are listed individually in the companion report.

## Complete object comparison

Each whole Core archive was walked through all 1,991 entries and its 1,988 COFF
members. The complete external-definition multiset has 149,470 rows in both
phases and compares equal after the recorded anonymous-namespace-name
normalization. All seven affected whole objects match their whole archive
members. Full consumer indices retain primary and raw auxiliary symbol records,
string tables, raw section headers and payloads, labels, and relocation records.
Each phase has seven BSS sections whose raw pointer is zero: their declared
sizes are preserved and their absent payloads are correctly treated as empty.

The seven-consumer comparison checks 204 complete function sections containing
19,203 bytes. Complete code and relocations compare equal. It also checks 346
complete sections for payloads, sizes, flags, labels, and relocations, excluding
`.debug*` and `.chks64`. All 122 excluded metadata-section records across both
phases remain fully retained; this packet makes no equality claim for them.

Four generated lambda identifiers differ between builds. They are paired by a
bijective mapping anchored to the same four relocations in two complete,
byte-identical parent functions: constructor relocations `0x253` and `0x26B`,
and `register_matrices` relocations `0x43` and `0x58`. The mapping then checks
every associated complete function body and relocation. No blanket lambda-name
normalization is used; all raw names remain available. Root and worker paths
and the header epoch differ, so the cause of generated-name drift is not
attributed solely to either factor.

Both complete constructor sections (1,868 bytes, 563 instructions each) and both
complete destructor sections (600 bytes, 192 instructions each) are decoded:

- Constructor `+0x60B` loads the manager-cell reference at observed access offset
  `0x34`; `+0x60E` reads its current value; `+0x611` calls AF0950 through relocation
  `+0x612` with the actual node. This order is unchanged.
- Destructor `+0x15D` loads the manager-cell reference at observed access offset
  `0x04`; `+0x160` captures its current value. The live-count store at `+0x16B`
  follows that read; `+0x173` calls AF0AE0 through relocation `+0x174`. This
  current-value capture and call order are unchanged.

These offsets are observations in the compiled Source objects. No exact total
interface-size or alignment claim is made. Whole-object, whole-archive, map, or
executable byte equality is not claimed, and the comparison is not Original
register-ABI, particle fixture, gameplay, or live model-path equivalence proof.

## Normal build and reproducible handoff

One normal `./scripts/build.ps1` invocation completed successfully. Its three
existing checks passed: `reconstructed_math`, `native_math_differential`, and
`tool_tests` (7.51 seconds total). The complete build log, zero exit result, and
LastTest log are retained. The verified existing native-math seed header stayed
unchanged. The existing math differential check executes its retained math
seeds; no particle Native body, new fixture, game launch, or runtime probe ran.
No new tests, C++ bodies, SDK substitutes, or Ghidra mutations were added.

The evidence directory is
`local/cc12_particle_model_typed_publication_views/`. Its standalone entrypoint
is `portable_replay.py evidence.zip`. It requires Python and Capstone; imports
and codec loading precede the guard. During replay, only the supplied ZIP may
be opened; external file reads, writes, subprocesses, networking, and dynamic
library loads are rejected. It reparses whole archives and consumer indices,
recomputes actual compiler cohort and Source evidence, verifies captured Git
object/tree/blob chains, repeats code/relocation and selected full disassembly
checks, and confirms the retained build results. It does not recursively open
Root proof trees or a live checkout.

Replay passed in place and from a physically copied ZIP and entrypoint under
`relocated/`, with identical result objects. `verify_git.py` performs the separate
actual Git/working-tree verification. The companion report pins the ZIP,
entrypoint, producer scripts, complete manifests, and all three replay/Git result
files. It records the selected artifacts' exact byte counts and SHA-256 hashes.
Source compilation and the portable proof are complete for this bounded type
change; additional model-owner and concrete-callee wiring needs its own
accepted readiness, Source epoch, ownership leases, and C++ authorization.
