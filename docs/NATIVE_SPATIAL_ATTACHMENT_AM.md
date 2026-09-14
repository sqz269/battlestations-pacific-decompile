# Raw unit-part spatial attachment

Addresses: `0070F7D0`, `00710AD0`, `00722C20`, `0098A310`, `0098A750`,
`0098AD60`, `0098B530`, `0098B920`, `0098BA10`.

The complete unit-part attachment path now has raw-storage source bodies,
including child, grid-root and loose-root registration. The new
`NativeUnitPartAttachmentBindings` extends the existing constructor storage,
group, collision, pooled-name and entry composition with this actual attachment
provider. It borrows the real publication cells and existing pose views; it
does not create a separate spatial registry or pose owner.

## Bodies and native interfaces

| Entry | Complete bytes | Native inputs and stack cleanup |
| --- | ---: | --- |
| `0070F7D0` | 101 | ECX part; EAX parent node; RET |
| `00710AD0` | 174 | ECX part; RET |
| `00722C20` | 121 | ECX accumulator; four float words; RET10h |
| `0098A310` | 190 | ECX index, EDX node; minimum/maximum pairs; RET8 |
| `0098A750` | 392 | ECX node; RET |
| `0098AD60` | 103 | ECX output pair, EDX point; RET |
| `0098B530` | 455 | ECX parent; child; RET4 |
| `0098B920` | 152 | ECX parent; child; RET4 |
| `0098BA10` | 316 | ECX index; node,parent,matrix,static word; RET10h |

Source adds a borrowed access pointer in unused EDX where available. Cell
conversion adds one stack access pointer and consequently uses RET4. Other
explicit source-ABI and borrowed-view boundaries are documented in the headers.
Native argument-stack aliases, register preservation beyond the documented
interfaces and original exception identities are not implied.

## Attachment and hierarchy

`710AD0` skips everything for any nonzero part `184` byte. Otherwise it queries
the live `164` owner's current `5C` virtual target for types `1C`, `1B`, `36`,
then `44`, stopping at the first nonzero AL. Each query reloads the owner and
target. It captures the owner before conditional pose refresh, then captures
that owner's matrix and the canonical `42E630` index before parent selection.
Only after the full `98BA10` call returns does it set part `184`.

`70F7D0` queries type `1E`, reloads owner `164`, then climbs parent `3C` links.
For each parent, a nonnull first `B0` virtual result triggers a second call
through the current target. Only the second result's `158` byte decides whether
that node is returned. Both calls and post-callback parent reloads are retained.
No cycle guard or recovery for an inconsistent null second result is added.
The two complete actual-unit virtual dispatch services remain required inputs;
native identity DWORDs are not invoked as fabricated rebuilt virtual tables.

`98BA10` captures node owner `4C` before writing the static byte. It refreshes
that pose when needed and copies its world matrix to node `50` through canonical
`4134F0`. It reloads owner `4C` for inverse handling, refreshes it when inverse
validity is zero, sets owner `10C` before canonical `B63D50`, then copies the
inverse into node `90`. Pure resolver lookups bind these operations to the actual
raw fields through existing `PoseRefreshView` and `PoseDerivedView` references.
The separate matrix argument is not read by the native body.

A nonnull parent takes `98B920`. At capacity, that routine publishes the wrapped
doubled capacity before checked multiplication by four and allocation. It copies
the wrapped count of pointer bytes, rereads/frees the old pointer, publishes the
replacement, appends the child, records its slot, increments the parent count,
publishes the child's parent and merges bounds. The recovered `98B976..98B97F`
fall-through after `_free` contains replacement publication and register/stack
restoration; its prior CALL_RETURN override was cleared under the write lock.
Allocation failure leaves the doubled capacity and original children intact.

`98B530` transforms eight child-bound corners into each ancestor's coordinates.
It multiplies child world by parent inverse inside every corner iteration,
preserves the original x87 comparisons and unordered branches, calls canonical
`98A920` for centre/extents and continues through current parent `108`.

For roots, `98BA10` clears parent `108`, rebuilds world bounds and maps both
corners into cells. Wrapped signed spans at most one in both axes select
`98A310` and insertion at the current root-list head. Larger spans append to the
loose array. Node `158` is set only after its branch completes. Capacity and
cell-range validity remain native caller invariants, not added checks.

## Numerical and intrusive details

`98A750` compares node `154` with the current published frame owner's `648`
DWORD. On a mismatch it retains the native x87/SSE argument spills, three
`722C20` absolute scaled accumulations, canonical `4142E0` centre transform,
world-minimum/maximum copies and extent subtraction/addition. A matching frame
stamp leaves the entire bounds region untouched.

`98AD60` divides X and Z by the current **double** at `CE3D90` (installed bits
`4079000000000000`, or 400), spills each quotient to float, widens it to double
for the required floor service, spills its result to float and calls canonical
`BF7420` against the current actual `0109EEA4` dispatch cell. The final addition
of 75 wraps. The floor service's original CRT dispatch and exceptional behavior
remain an explicit boundary; no general CRT floor implementation is invented.

`98A310` retains inclusive X-major/Z-minor iteration, all current-head reloads,
the repeated previous-pointer zero store, untouched link-owner backpointers and
the shift/add packed key. These raw bodies coexist with the older semantic
projections in `spatial_index.cpp`; the older projections are not substituted
for the raw attachment path.

## Verification and remaining work

Nine complete original bodies, totaling 2,004 bytes, match fresh saved-Ghidra and
installed-PE captures. All 29 direct call rows and seven indirect virtual-call
sites are retained and checked. The Win32 build and both existing CTests pass.
No repository tests were added.

One ignored probe compares 60 paired original-byte/rebuilt cases: five complete
unit-part attachment scenarios, 36 cell conversions, 18 world-bounds cases and
one allocation-failure path. The numerical cases cover six x87 control words,
both integer-conversion dispatch branches, NaNs, infinity, subnormal values,
signed zero, cached frames and an output/point alias. Attachment cases cover
four-cell head insertion with an existing root, loose append, child growth and
ancestor bounds, the nonzero attached guard, and live unit-owner replacement.
The child-parent fixture changes its B0 target between the two required calls.

The allocation case passes an impossible FFFFFFFF-byte request to the actual
source CRT allocator through both the copied EH-free callers and rebuilt path.
It verifies retained capacity and child storage, unchanged parent publication,
and unset attachment flags after `bad_alloc`. This is source-CRT exception
behavior, not proof of original native FH3 or exception-object identity.

The original reference and rebuilt side share canonical pose, matrix, local
bounds, index publication and source CRT/SDK boundaries. The fixture supplies
real callable unit targets in its own tables and an SDK floor binding; these
are explicit fixture services, not evidence of actual game-unit dispatch or
original floor internals. Pointer identities are normalized in the retained
whole-storage images; native/source branch traces and floating status agree.
The constructor's new attachment override is executed, but this packet does
not claim another complete constructor comparison with nonempty collision data.

Compiled caller disassembly confirms volatile owner/target reloads, the captured
pose and index, duplicate parent calls, capacity-before-allocation, canonical
providers, all three branches and final flag timing. The source access-stack
adjustments and cleanup of the numerical kernels are inspected separately.
The actual executable still needs spatial ownership admission, complete unit
dispatch and floor bindings, detachment/destruction, and a populated refresh
registry. Original FH3/fault delivery and gameplay parity remain unproved.
