# Grid canonical allocation/free defaults

The default grid CPU allocator now calls
`singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes})`,
and the default outer-grid free calls `singleton_lifetime_free`. The existing
internal-buffer `BF6989` service remains `std::free`. This pairs the six internal
buffers and the canonical outer `84h` allocation with their matching source
free families.

The source change is exactly the two `NativeGameGridCpuCalls` method bodies,
one direct singleton-lifetime include, and four adjacent header comment lines.
Signatures, exception specifications, virtual calls, zero-size forwarding, six
independent requests, live field reads, post-loop clearing, scalar flags,
numeric tables, publications and failure/diagnostic schedules are unchanged.
Foreign allocation domains still require the corresponding matching virtual
free override. No raw-grid table, adapter, owner, registry or activation was
added.

## Source and build boundary

The baseline is Main `4d0ae310d6a85a5962fa5e262b1003a43e4bbd26`, which contains
the independently accepted grid readiness packet. Its existing retained proof
has 79 pins; every pin was checked unchanged before this implementation and in
the final original-worktree replay.

Three complete worker cohorts are retained: the initially stale build, a fresh
normal baseline build, and the normal post-change build. Each captures all
4,208 tracked `src/` and `include/` files as committed Git bytes and actual
working bytes, every actual Core/App object, the complete Core archive, actual
App executable and link map, test executables/logs, build project/command data
and compiler metadata.

The initial cohort had 1,988 Core objects. The fresh baseline adds the accepted
locale-node object and refreshes three particle objects, reaching 1,989; all
80 App objects were unchanged in that baseline step. These pre-existing source
updates are not attributed to the grid correction. Fresh-before versus after
is the implementation comparison.

Both normal `scripts/build.ps1` runs passed the three existing checks:
`reconstructed_math`, `native_math_differential` and `tool_tests`. No test cases
or probe executables were added. The already verified 2,995-byte seed header was
reused unchanged, with its historical verification receipts retained. The
existing differential check runs eight earlier math seeds; it is not an
original-grid allocator test. There were zero fresh Native queries, Ghidra
writes, ledger edits, OS probes or game runs.

The captured compiler is MSVC `19.51.36244.0`, X86. These are rebuilt source
results, not a claim of original allocator ABI equivalence or runtime grid
validation.

## Complete compiled methods and providers

| Method | Before | After | Verified boundary |
| --- | --- | --- | --- |
| Grid CPU `allocate_00bf55be` | 18 bytes, seven instructions, relocation to `operator new` | 60 bytes, 20 instructions, canonical allocation plus MSVC stack-cookie symbols | Both size fields copy the original stack argument, including zero; kind is `object` (`3`). Complete body and every relocation operand retained. |
| Grid CPU `free_00bf65ac` | 18 bytes, seven instructions, relocation to `operator delete` | Same 18 payload bytes and seven instructions, relocation to canonical free | The argument is forwarded directly and the method retains `RET 4`. |
| Grid CPU `free_00bf6989` | 19 bytes, seven instructions, `__imp__free` | Identical complete body and symbolic relocation | Preserved source and compiled internal-buffer free. |

MSVC emitted the allocation wrapper's stack-cookie guard around the addressable
12-byte request structure. That compiler-generated expansion is retained and
decoded in full. It is not reported as an 18-byte relocation-only change.

The two new canonical targets have unique positive definitions among all
1,989 actual Core objects, in `singleton_lifetime.obj`; that whole object is
unchanged. Canonical allocation is a complete 90-byte, 37-instruction body with
malloc/new-handler/bad-allocation boundaries. Canonical free is a complete
six-byte indirect jump to `__imp__free`. The corresponding current source
implements `std::malloc` with `_callnewh` retry/throw behavior and `std::free`.
No assumption that C++ and CRT allocation families are interchangeable is used.

All Core archive object members match the actual captured object files in each
cohort. The fresh comparison contains:

- **Core:** 1,985 whole objects identical; four changed. Three changes are only
  the raw COFF timestamp. In `native_game_grid.obj`, exactly the two intended
  function bodies/relocations differ; the other 55 functions are preserved.
  Across all four changed Core objects, 266 unrelated functions retain their
  complete payloads and symbolic relocation operands.
- **App:** 78 whole objects identical; two changed only their COFF timestamp.
  All 844 functions in those two objects are preserved, as are all their
  section payloads and symbolic relocation operands.
- **Grid sections:** the two method sections change as described above.
  Eight bytes at offsets `160..167` of the non-executable `.chks64` section
  also change. Its complete payload is retained without assuming undocumented
  checksum semantics. Every other grid section is unchanged under the stated
  payload/relocation comparison.

Thus 1,110 unrelated functions in recompiled objects are positively preserved.
Whole-object equality covers the remaining objects. This does not imply that
all functions retain the same final linked addresses.

## Actual linked executable

The before/after executable size is 5,102,592 bytes, but **1,026,876 actual bytes
change**. No timestamp, address or relocation normalization hides those changes.

| Section | Actual changed payload bytes | Qualification |
| --- | ---: | --- |
| `.text` | 958,580 | Method changes and linked layout/relocation differences; no global linked-code identity claim. |
| `.rdata` | 8,870 | Includes resolved table/other pointer and metadata differences. |
| `.data` | 0 | Whole payload identical. |
| `.rsrc` | 0 | Whole payload identical. |
| `.reloc` | 59,421 | Actual relocation-section differences retained. |

The five remaining changed bytes are classified by their exact PE fields:
COFF timestamp, entry-point RVA, base-relocation directory size, `.text`
virtual size and `.reloc` virtual size. The entry point and `.text` virtual
size each decrease by 32; the relocation directory/virtual size decreases by
16. The report keeps complete byte-difference ranges and both PE summaries.

Ten complete linked bodies are proven against the corresponding Source COFF
bodies: both stages of the two corrected methods, `BF6989`, and both canonical
providers. Each non-relocated byte matches, and every relocated operand resolves
to the symbol's actual link-map address. In particular, the unchanged canonical
allocator object has a changed linked throw-call displacement because
`__CxxThrowException@8` moves from `103AD816` to `103AD7F6`. The canonical free
and `BF6989` linked bodies remain byte-identical.

The existing **CPU-service C++ table** is also checked in the actual executable:

| Slot | Before VA | After VA |
| --- | --- | --- |
| Scalar destructor | `10014810` | `10014810` |
| CPU `BF6989` | `102DB870` | `102DB8B0` |
| CPU `BF55BE` | `102DB850` | `102DB850` |
| CPU `BF65AC` | `1015C150` | `102DB890` |

This table is not the raw grid's numeric `00CFD484` primary table. Callable
raw-grid table ownership and game construction/activation remain unadmitted.

## Retained epochs and replay

The previous accepted Root Source3354-particle epoch remains a separate whole
manifest and archive. Its selected 3,354 inputs match the fresh worker baseline
after line-ending normalization; only the two grid source/header files differ
after this implementation. Its four old build artifacts receive no fresh or
unchanged-build credit.

The new worker epoch is
`local/cc12_grid_canonical_default_pair/Source3354_grid_canonical_default_pair_frozen.json`:
3,354 selected inputs plus the four actual post-build artifacts. Complete
worker source coverage remains 4,208 files in the three cohort captures.

`reports/cc12_grid_canonical_default_pair.json` pins the finite retained bundle,
the complete compiled/linked audit and the portable verifier. Replay verifies
the entire receipt; the tracked report omits only the bulky archive member
indexes and byte-range arrays from its inline summary. Those records remain
whole in the pinned audit. Portable replay, an actual Git/working-source check,
and a physical relocation replay are recorded separately.
