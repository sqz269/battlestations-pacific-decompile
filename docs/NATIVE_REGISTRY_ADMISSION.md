# Host registry binding admission

Addresses: none. This implements the host registration contract approved in
`docs/NATIVE_COCKPIT_REGISTRY_ADMISSION_BH.md`. It adds no reconstructed native
body, native call row, original ABI replacement, or native reference operation.

`SceneAttachmentRuntime` and `GeneratedModelLifetimeRuntime` each expose their
own nested move-only `BindingAdmission`, `reserve_binding()`, and admitted
`bind(node, std::move(admission))`. A token contains only its concrete runtime
pointer. It neither names nor owns a node, native slot, vector element, or
callback. Existing bind/unbind call signatures remain available.

For the existing node vector, let L be its size, P the pending-credit count, and
C its capacity. Every completed operation preserves `L + P <= C`. Preparation
and fresh ordinary binding with outstanding credits check `P < max_size() - L` before computing
`L + P + 1`, and reserve that capacity before changing either entries or P.
Successful preparation increments P and returns its nonthrowing token. Ordinary
bind preserves P. With P=0, ordinary binding uses the original vector append
and its normal growth/maximum-size checks; it does not force a reserve of L+1
for every insertion. A successful admitted append consumes one P and empties the
caller's token. The attachment-link vector receives no admission accounting.

Validation precedes any admitted mutation. The scene overload shares the old
raw-key/runtime and transform/key checks; rebinding the same companion cancels
its unused credit without appending. The lifetime overload shares the old
scene/transform and duplicate-transform checks, then additionally requires the
exact attachment returned by the existing scene runtime's `resolve_key` and
rejects any existing lifetime with that actual key. These extra checks apply
only to admitted binding. No uninitialized native scene word is read.

The active token proves P is at least one, hence L is less than C. Appending one
pointer with the existing standard allocator needs no allocation or throwing
element copy. No getter or callback lies between that append and credit
consumption. Successful admission with valid preconditions is nonallocating;
diagnostic exceptions may allocate and deliberately are not `noexcept`.

| Static review case | Result and source evidence |
| --- | --- |
| Overflow or reserve allocation failure | Subtraction checks the length before addition; pointer-vector reserve precedes all state changes. Entries, P and tokens are unchanged on failure. `scene_attachment.cpp:293-303`, `generated_model_lifetime.cpp:65-74`. |
| Empty, consumed or wrong-runtime token | Runtime-pointer check precedes all node identity reads; an exception leaves the caller's token active if it was active. Scene lines 324-328; lifetime lines 88-92. |
| Rejected identity | Shared legacy checks and admitted lifetime canonical/key checks precede append and credit consumption. Scene lines 305-336; lifetime lines 76-102. |
| Move construction and assignment | Pointer transfer empties source; self-assignment is a no-op; other assignment first cancels the destination's credit, including cross-runtime assignment. Scene lines 270-288; lifetime lines 42-60. |
| Cancel, unbind, forget | Cancel decrements exactly one active credit and clears the token without vector/native access. Existing unbind/forget bodies are unchanged and do not affect P. |
| Runtime teardown | Copy/move is deleted. Nonthrowing destructor terminates only for outstanding credits and does not visit entries or detach nodes. Scene lines 290-292; lifetime lines 62-64. |

Use one serialized owning thread and stable runtime addresses. Active tokens
must not outlive their runtime. Identity getters must remain stable,
nonallocating leaf accessors; registry mutation and allocator/new-handler
callbacks must not reenter a registry operation. Callbacks between completed
operations can use their own prepared credits. An ordinary callback bind can
allocate to preserve other credits, so a larger allocation-free native sequence
must prepare every required binding in advance.

The accepted terminal ordering remains unchanged: native scene forgetting,
concrete pool return, lifetime unbind, companion disposal. The approved design's
source review establishes that concrete serialized raw-slot return has only
Win32 critical-section operations, arithmetic and stores, with no allocator or
host callback. Normal reuse after terminal release remains supported. This
implementation neither adds a reuse restriction nor changes that provider.

## Verification boundary

Worker base: `452ff94dffadcf9976ccea4b850e39e4f7f6bc1b`, branch
`agent/orch3-registry-admission-impl-bh`. The fixture build uses that complete
worktree plus exactly the four source changes in this packet, identified by
SHA-256 in `reports/native_registry_admission.json` and the external input
manifest. This is a working-tree build boundary, not a claim that the unchanged
base commit contained admission. The final external manifest records the
resulting commit after verifying those same source bytes.

The runtime layout changes. All translation units and all three linked static
libraries are built together with the current worker headers. No fixture links
new headers or new translation units against earlier-layout baseline libraries.

Fresh MSVC Win32 Release build through `scripts/build.ps1` passed with the
existing strict core flags (`/W4 /WX /fp:strict`). Both existing CTests passed:
`reconstructed_math` and `native_math_differential`. All eight prerequisite
seed byte ranges matched; no unowned source edit was needed for compilation.

The external probe passed: two ordinary host allocations in each registry,
zero attempted host allocations while admitting the surviving credits,
canonical identity lookup, and explicit cleanup. All 2,451 source/header/build
input hashes matched before/after the whole build. All 2,604 frozen fixture
inputs, all three current worker libraries, the executable, and all 292 included
headers remained unchanged across the fixture. The probe uses `/MANIFEST:EMBED`.

The read-only external ZIP contains frozen sources and headers, dependency
inputs, the three fresh libraries, probe source/executable/recipes, seed evidence,
build/CTest/probe logs, and before/after manifests:
`J:/PROG/bsp-evidence/orch3-registry-admission-impl-bh-20260913.zip`.
SHA-256:
`76970190A5793AC401FD72114C4A8749790243AE78CCCFFA68689DC499B16775`.
The final committed-source check is separately sealed at
`J:/PROG/bsp-evidence/orch3-registry-admission-impl-bh-20260913.manifest.json`.
The integrator must still build its combined tree with its own matching headers.

### Frozen and current-tree replay

For frozen replay, extract the ZIP into a new external directory. Copy
`run_probe.cmd`, change its `cd /d` target to that extraction directory, and run
it with the recorded MSVC x86 toolchain. Its relative include/library arguments
then use the extracted frozen inputs. Keep the original ZIP unchanged and
record the adjusted recipe and new executable/log hashes separately.

For a current-tree replay, first run `python tools/ghidra_export.py verify-seeds`
and `scripts/build.ps1` from that selected root. Copy the archived
`registry_credit_probe.cpp` and recipe into a new external output directory,
change the recipe's `cd /d` to that directory, and replace its input paths as
follows, retaining the recorded compiler flags and `/MANIFEST:EMBED`:

| Frozen recipe argument | Current-root counterpart |
| --- | --- |
| `inputs/include` | `<root>/include` |
| `dependencies/lua511-src/src` | `<root>/build/win32/_deps/lua511-src/src` |
| `dependencies/zlib121-src` | `<root>/build/win32/_deps/zlib121-src` |
| `dependencies/dxsdk_d3dx-src/build/native/include` | `<root>/build/win32/_deps/dxsdk_d3dx-src/build/native/include` |
| `libraries/bsp_core.lib`, `libraries/bsp_lua511.lib`, `libraries/bsp_zlib121.lib` | The corresponding three files in `<root>/build/win32/Release` |

Run the newly compiled executable. Record the current root/commit, all input
hashes, three library hashes, adjusted recipe, executable and logs before/after.
The archived `verify_registry.ps1` and `verify_probe.ps1` intentionally pin this
worker's absolute paths; they are evidence recipes, not current-root selectors.

The external fixture is one registration-only credit-isolation scenario applied
to both concrete registries: reserve two credits, perform two ordinary binds
that consume the original spare capacity, move/cancel one token, then forbid
host `new` while consuming the survivor. It checks canonical key/transform
identity and explicitly unbinds all entries. Its diagnostic companions use their
stable Win32 attachment addresses as keys. Virtual behavior callbacks terminate
if invoked; they supply no native-behavior evidence. No repository test or test
framework was added. The other cases above are static-review evidence.

The worker checklist's callee, function-boundary, call-site, native layout and
runtime-attribution rules introduce no new native claims here. Ghidra is
read-only for prerequisite seed verification, against configured `bsp.gpr` and
`/battlestationspacific.exe`; no annotations or ledgers change.

Camera owner/reference admission overloads remain a later reviewed packet.
Persistent cockpit/viewport companions, their publication and retirement,
native constructor/EH composition, original ABI compatibility and game
validation remain outside this host registration implementation.

## Integration correction

The worker archive and its four source hashes above retain the original reviewed
worker build. Integration changes only the two ordinary bind sites to call the
capacity helper when P is nonzero. This preserves the existing vector growth
policy when there is no pending credit; unconditional reserve(L+1) would instead
allocate at each ordinary insertion. Reserved/admitted paths and validation are
unchanged. The exact corrected combined source and current-library fixtures are
recorded in `reports/native_cockpit_admission_bh_validation.json`.
