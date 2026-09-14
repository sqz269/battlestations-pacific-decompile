# Actual descriptor sampler append

Addresses: 00b3b280

`append_native_material_descriptor_samplers_00b3b280` implements the complete
normal B3B280..B3B3B1 schedule over the existing actual builder, pass, descriptor,
sampler, effect, pooled names and canonical retained-owner domain. Descriptive
names are hypotheses. Its added context and persistent failure frame give it a
new C++ ABI; it is not a drop-in native function or full compiler/game result.

| Routine | Original ABI | Coverage |
| --- | --- | --- |
| B3B280 | ECX actual B0h builder; stack actual 88h pass, actual 110h descriptor; RET8 | Complete normal schedule, with explicit native stack preimages and existing provider-domain limits |

The builder is `NativeMaterialProgramBuilderStorage`, not the semantic shader
builder. The existing opaque bytes at 8C/90/94 are read and incremented in place
as wrapping DWORD counters. Actual descriptor C4/C8 and sampler 0C/14/18/20/24
are reused from their existing producer layouts. No duplicate manager, projected
cache, copied reference count or replacement loading callback is introduced.

Source zero calls B5F100 with current index20 and the actual stage byte, then
increments current builder8C. Source three first calls B18FB0 on current
builder78 effect, then rereads sampler20 for `FFFFFFFF-index`, appends the route
and increments8C. Source one calls actual 4DE4B0 and the full actual B1B4D0 entry
in [NATIVE_SAMPLER_CACHE_ENTRY.md](NATIVE_SAMPLER_CACHE_ENTRY.md). Null skips the
binding and8C increment. Nonnull captures the returned owner, calls B44CF0 with
current builder8C and actual name18, writes pass80=1, decrements captured owner+4
and invokes its current virtual0 only on zero through `NativeRenderActualOwners`.
Only after that release returns does8C advance. Types two and unknown types skip
texture work; every type still processes states and advances a stage counter.

The unsigned outer loop reloads descriptor data/count at each native boundary.
Each sampler pointer stays captured during its iteration. Each state iteration
reads the current vertex byte, current90+16 or94, current list data and row;
B5ED60 is followed by a reload of sampler24 and count. The final vertex-byte
read chooses which current counter advances. Native effects completed before a
source exception remain visible; no rollback or retry is added.

The native stack detail is observable. Let B be B3B280's ESP after its saved
registers. B5F100's four-byte stage local is B-16. It writes only byte zero and
later copies all four current bytes; the initial word is an explicit borrowed
input. After source three, B18FC9's saved caller ESI is the last writer of B-16,
so the sampler identity supplies the upper bytes. B5ED60's three-argument CALL
writes its return PC B3B36E there. A returning B1B4D0 leaves its CBC760 handler
word there. A returning B44CF0 leaves B3B2E4 there, but the following imported
atomic and optional owner virtual0 can overwrite it. Each successful source-one
release therefore consumes one explicit post-release residue row carrying
sampler ordinal, captured owner identity and DWORD. Consumption/validation is
after the real release returns. Missing or mismatched input throws at that
point, preserving preceding effects. Later proven writes still replace it.
Whole-routine native scratch equivalence requires matching captured residues;
the implementation does not invent OS/profile-specific stack bytes.

The one-shot operation owns a stable list of per-sampler host frames. Singleton,
default-loader, cache continuation and leaf frames are adopted before calls.
A loader failure leaves its actual requested/default headers and inner
continuation acquisitions alive. No production catch acknowledges a child or
returns a fallback owner. Discarding unresolved frames terminates. Borrowed
globals, pools, contexts, sampler rows and acquired owners must remain alive;
external retirement while running/failed is excluded. Native FH3, hardware
fault and exception identity equivalence are not implemented.

Validation: verified BSP project/program and installed PE; 8 seed checks;
strict default MSVC Win32 build and both existing CTests passed. One ignored
fixture compares 2,869 relocated normal-body bytes across nine original bodies
with the source composition. Its nine-sampler schedule covers source0,
source1-hit/null, source2, source3 and unknown types; five actual route rows;
live descriptor/list/stage/counter mutation during real allocation; pooled
Default loading; and actual resource references ending at2. Original and source
share the substantive reserve/string/cache children and an explicit atomic
bridge. The bridge records the relocated original post-release residue used by
the source leg; this is not a Windows atomic implementation comparison.

Source-only failure checks retain all nested frames/names, preserve completed
parent effects, reject replay and enforce destruction guard exit77. The real
Win32/XLive pump runs only in the isolated fixture's native null-online domain.
Nonempty raw online/input/cursor execution, terminal resource release, original
FH3 unwinding, compiler-parent integration and gameplay are unproved. Native
writer/profile/FH3 context, exact source hashes, compiler/read dependencies,
link selection and loaded runtime files are frozen in the archive referenced by
[the report](../reports/native_material_descriptor_samplers.json).
