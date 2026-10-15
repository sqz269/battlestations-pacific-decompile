# Descriptor Source1 observed release (CC12)

This packet implements the conditional nonzero-release producer established in
[the preceding contract](NATIVE_SAMPLER_SOURCE1_RELEASE_CC12.md). It records the
actual atomic result and qualifies the exact current Windows provider before
publishing `W=B-16=00B3B2E4`. It does not establish a real loader-owner
Original/Source execution comparison. Runtime admission remains open.

The field is descriptor `sampler+14h == 1`. Compiler `source_mode=0` and
`load_variants=0` remain unchanged. The cached compiler's 96-byte COM release
bridge and its immutable evidence families are unchanged. No native payload
layout, original assembly, numeric method table or Ghidra annotation changed.

## Source behavior

`release_native_render_actual_owner_observed` adds a one-shot retained
observation to the canonical release behavior. The existing public
void release helper keeps its signature and delegates to it with a local
observation. The observed helper:

1. Records the same owner identity in disjoint, caller-retained metadata.
2. Performs exactly one sequentially consistent `fetch_sub(1)` on actual
   owner `+04`.
3. Records the actual prior count and the modulo-32 resulting bits derived
   from that atomic result. It never rereads the count to infer the branch.
4. If the prior count is one, enters the unchanged canonical zero dispatch:
   resolve the current companion, verify its borrowed count is the same actual
   `+04`, and call its current terminal. Afterward it touches only observation
   metadata; it does not inspect the retired owner or companion.
5. Records normal return. A throwing resolution/identity check leaves the
   already-zero obligation and entered/decremented state in the retained frame.

Observation storage must be separate from owner/companion storage and survive
the terminal. Reusing an entered observation is rejected before another
decrement. No new owner registry, retain, count rewrite, destructor substitute
or cleanup policy is introduced.

Each descriptor Source1 step now retains that observation, sampler index and
pointer, the actual invocation identity, before/after provider observations,
and whether the word was established. The actual builder/pass/descriptor and
context remain in the surrounding one-shot operation. The production sampler
factory supplies its retained CallScope identity to both root and mode; its
nonexecuting domain view supplies null. Existing aggregate contexts default
to null and retain their previous explicit-capture behavior.

After the ordinary binding return, the source still records its established
native return word. Following release, automatic knowledge requires all of:

- A retained, nonnull invocation identity and normal observed release return.
- An actual atomic prior count greater than one.
- Exact matches of the complete current nonforwarded Kernel32 atomic provider
  before and after the release, with the same module and export addresses.
- The unchanged Original `B3B2DF` three-argument call placement, whose return
  slot is B-16 and return PC is `00B3B2E4`.

The qualifier checks DOS/PE32/I386 identity, timestamp `0F052F7B`, image size
`000F0000`, export RVA `00031AB0`, and all 20 provider bytes. Its profile bytes
equal the existing cached-entry profile. It calls no release and creates no
D3D device. The original helper's only push reaches B-12; therefore the
qualified nonzero path cannot overwrite the prior binding word at B-16.

Release consumption first marks shared knowledge unknown. It restores
knowledge only from the established word or an existing matching explicit
capture. A supplied capture must identify the same sampler and released owner;
when automatic proof exists, its word must also agree. An identity/order/word
failure cannot leave the stale pre-release word advertised as known. Sparse
later captures remain unconsumed until their own sampler. Unsupported provider
or zero-terminal paths receive no automatic word; independently captured
residue and later native state writes retain their existing contracts.

## Verification and readiness boundary

The report records the strict MSVC Win32 build, existing check results and
exact source/header/object receipts. It distinguishes the read-only current
provider probe from any release invocation. No synthetic LONG/owner test is
used as a substitute for the requested actual resource comparison.

The Source helper continues to use its canonical inline atomic. Profiling the
Windows export qualifies the Original imported helper's stack footprint; the
Source does not copy its own C++ stack or equate that stack with native W.

Compiled-code inspection verifies one `LOCK XADD` in the observed helper,
prior/result stores before zero dispatch, same-companion identity validation,
and metadata-only writes after the terminal. The legacy public helper remains
byte-identical to the frozen `f0837786` baseline: 78 bytes and four relocations,
including equality of resolved in-object target-section bytes. The report
also pins the Source1 branch, consumption and provider qualifier sections.
This is generated-code evidence, not original private-caller ABI equivalence.

The requested real-owner fixture was not executed. Source1's loader selects
the CE7D24/B1B810 procedural registry route (Caustics/ShoreWave factories),
whereas the available texture fixture's B319B0/`black.tga` owner belongs to a
different cache. Injecting that texture owner or pre-populating a cache would
not prove the actual Source1 path. No registry/cache seed or fabricated owner
was introduced.

The next fixture must first establish a genuine populated procedural registry,
using the same live raw manager/registry and canonical BBC5F0/BBC740 or BBC900
construction/registration with complete actual name/registration frame
bindings. That preparation is a separate readiness requirement, not evidence
that World or `fakemotionblur.mshd` reaches descriptor Source1. The primary
integrator also reported existing actual D3D device-creation failures before
its texture fixture body; this worker did not repeat an unchanged GPU launch.

After those prerequisites, run the original-address B3B280 parent with actual
loader-returned owner, native binding and a following Source0 reference.
Observe the real imported decrement result and B-16 without a stack-writing
shim. Compare the current Source result, actual identities/counts, binding,
row DWORD and counters; freeze all original/source children, callbacks,
providers, code/headers, COFF objects and rootable call coverage. Zero-terminal
and malformed-capture execution remain separate unverified paths. Preserve
preparation failures and stop on observation timeout rather than relaunching.

No full Original/Source release fixture, complete source/provider runtime
closure, linked application activation proof, original FH3/SEH or whole caller
ABI, startup or gameplay credit is claimed. The generated implementation is
available for integration review with that runtime boundary explicitly open.

See [the report](../reports/native_sampler_source1_observed_release_cc12.json)
for the frozen family, checks and exact remaining frontier.
