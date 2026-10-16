# Private qsort valid-input Source kernel

Addresses: `00BFC350`, `00BFC2C0`, `00BFC290`.

This provides an ordinary MSVC Win32 byte-sort kernel using the actual passed
comparison function and actual element addresses. Entry coverage is explicitly
**partial_valid_input_entry_projection**. Two private helpers cover their whole
normal geometry through new C++ interfaces. Correct Original library names
`_qsort`, `shortsort` and `swap` remain unchanged.

| Original entry | Retained body | Source coverage |
|---|---|---|
| `00BFC350` | 676 owned bytes / 230 instructions, two ranges | partial valid-input entry projection |
| `00BFC2C0` | 132 owned bytes / 55 instructions, two ranges | whole normal helper geometry |
| `00BFC290` | 40 bytes / 20 instructions | whole normal helper geometry |

The qsort body owns `[00BFC350,00BFC53D)` and `[00BFC540,00BFC5F7)`;
shortsort owns `[00BFC2C0,00BFC2DD)` and `[00BFC2E0,00BFC347)`.
Each three-byte gap is an unowned `LEA ECX,[ECX]`, not part of either body.
Complete retained instructions were read and decoded before Source dispatch.
That evidence does not make the new entry a whole native reconstruction.

## API and admitted domain

`NativeQsortCompare` is `std::int32_t (__cdecl *)(const void*, const void*)`.
`sort_valid_native_bytes_00bfc350(actual_base, count, width, compare)` is a
potentially throwing ordinary C++ API. The passed function receives addresses
inside the actual allocation; no pivot copy, comparison adapter or host is used.
The native qsort entry is cdecl with four DWORD stack arguments and plain RET;
the new interface has no physical Original ABI or exception/fault equivalence claim.

For every count, width is positive, compare is a genuine nonnull compatible
function, and base is nonnull whenever count is nonzero. Original validation
precedes its unsigned count-less-than-two return. This implementation represents
those successful validity conditions as caller preconditions. Count zero or one
returns without element access, comparator invocation, extent multiplication or
sort-helper work. A zero count permits null base only within the other preconditions.

For count at least two, the live writable allocation contains `count*width`
bytes of actual byte or trivially-copyable/byte-relocatable element storage.
Elements are disjoint and every reached address meets the genuine comparator's
type, alignment, representation and lifetime requirements. Require the mathematical
product `count*width <= INT32_MAX` and nonwrapping actual base-plus-extent, including
its one-past address. This conservative bound is a Source admission restriction,
not a recovered native overflow check.

The comparator returns normally and defines a coherent stable strict weak ordering
over the logical element values. No callback mutation, reentry, throws, faults,
overlap/volatile/MMIO protocol, concurrency, teardown, locale or lifetime changes
are admitted. Private volatile byte accesses preserve the specified exchange
order; they add no synchronization. No fallback algorithm or new runtime guard
repairs inputs outside this domain.

## Preserved normal schedule

Subranges use inclusive low/high endpoints. At most eight elements enter shortsort;
larger ranges select `mid = low + floor(count/2)*width`. They always compare
`(low,mid)`, `(low,high)`, `(mid,high)` in that order and exchange each pair only
for a signed positive result. The pivot remains a live element address.

The low scan advances before comparison. Its first loop handles addresses below
pivot; reaching pivot skips self-comparison and enters the second incrementing
loop. The second loop checks high before comparison. Both continue on a signed
result at most zero. The high scan decrements first, stops at or below pivot
without comparison, and otherwise continues only on a signed positive result.

While low-scan is at most high-scan, exchange distinct elements. If high-scan was
pivot, move pivot to low-scan, including the equal-address no-exchange case.
Continue from the retained scan positions. After the scans cross, advance
high-scan once and use two distinct predecrementing equality scans: above pivot,
then below pivot. Skip pivot, compare only above original low, continue exactly
on zero, and stop at low without comparing it.

Compare signed byte distances `high_scan-low` and `high-low_scan`. Empty right
has distance `-width`; the wider signed Source arithmetic preserves this result
without unsigned underflow or invalid pointer subtraction. Strictly smaller left
means push right and process left; otherwise push left and process right.
Equal distances therefore push left. Schedule only partitions with at least two
elements. Two fixed 30-entry endpoint arrays preserve matched low/high pairs and
LIFO order. No recursion or dynamic worklist appears.

Shortsort repeatedly starts maximum at low, scans `low+width` through current high
in ascending addresses, compares `(scan,maximum)`, and replaces maximum only on
a signed positive result. Exchange maximum with high if distinct, then decrement
high. A normal k-element range performs `k*(k-1)/2` comparisons even when ordered.

The three distinct exchanges visit bytes in ascending order:

- Median helper: read second, read first, write first, write second.
- Partition inline loop: read low, read high, write low, write high.
- Shortsort inline loop: read high, read maximum, write maximum, write high.

The actual Original direct helper calls are:

```text
00BFC40E CALL 00BFC2C0
00BFC45F CALL 00BFC290
00BFC47A CALL 00BFC290
00BFC495 CALL 00BFC290
```

The qsort indirect comparison sites are `00BFC44B`, `00BFC466`, `00BFC481`,
`00BFC4AC`, `00BFC4CA`, `00BFC4E0`, `00BFC548`, `00BFC56C`; shortsort's is
`00BFC2F2`. Each receives actual right then left stack arguments and caller-pop8,
with signed EAX results. There is no comparator sign normalization or stable
tie-breaker. Symbolic branch reasoning shows two equal shortsort elements swap,
an all-equal shortsort range rotates identities left once, and an all-equal range
larger than eight undergoes no exchanges. These are static deductions, not fixtures.

## Arithmetic, stack and coverage limits

Source scan state and worklists use bounded unsigned offsets. Only actual reached
element/byte accesses form pointers. The one-past low-scan sentinel is never
dereferenced. The admitted extent makes unsigned offset comparisons agree with
native unsigned addresses; signed partition differences lie between `-width`
and `(count-1)*width`. Width, size and pointer comparisons are not replaced by
signed arithmetic; only the native signed comparator/distance/exhaustion decisions
use signed interpretations.

Crossed scans and equality trimming yield disjoint children with combined size
at most their parent and with each child strictly smaller. Each continued descent
after a push processes at most half its parent. A chain of k pending pushes needs
at least `9*2^(k-1)` elements because only ranges larger than eight partition.
With count at most `2^31-1`, k is at most 28. Immediate absent/singleton children
pop without another descent. The native 30 entries therefore suffice under the
stated domain. This is a static proof, not an executed stress test.

Original failure blocks `[00BFC36E,00BFC394)` (38 bytes / 13 instructions) and
`[00BFC3AA,00BFC3D1)` (39 bytes / 14 instructions) remain excluded Source behavior.
They cover errno22 and private invalid-parameter delivery. Successful validation
predicates are caller preconditions, not reconstructed runtime checks. Original
thread-data/fallback ownership and private validation return/unwind/fault behavior
remain external. This kernel needs no errno, invalid-parameter, TLS or global service.

The actual scene wrapper, both scene comparator boundaries, F89B75 data-role
lifetime, narrow-locale bindings and application ownership remain separate.
Existing scene `std::qsort` use is not changed by this isolated kernel.

## Worker verification

The single MSVC Win32 strict compilation passed with `/W4 /WX`; all ten
compiler-declared inputs match this phase's pins captured before compilation.
The one existing baseline build passed both configured tests (`reconstructed_math`
and `tool_tests`). Neither check executes the unregistered kernel.

The complete 2,527-byte COFF was reviewed: six sections, 13 primary symbols plus
six auxiliary slots, all relocations and the complete string table. Its three
code sections contain 1,368 bytes / 424 decoded rows. The public kernel is
1,191 bytes / 342 rows and inlines the helpers; separately emitted private
shortsort and swap copies are 129 bytes / 59 rows and 48 bytes / 23 rows, and
are unused by the public body. Ten NOP rows occupy 44 bytes across the object;
every other emitted code row was read, including the unused helper copies.
The only external symbols are compiler security-cookie support, with one
DIR32 and one REL32 relocation. No EH tables or RTTI are emitted. Original
instruction/fault ordering and physical ABI/EH compatibility remain unproven.

Detailed validation results and exact input/object hashes are in
`reports/native_qsort_kernel.json`; the own task evidence folder is
`local/WholeNativeQsortKernelSource01`. The new translation unit remains
unregistered pending integrator review. No target-body execution, new tests or
probes are authorized by this packet. Existing baseline build checks do not
execute the unregistered body. Source completeness on the admitted domain,
compilation, whole-object review, Main registration/archive binding and runtime
proof are distinct gates.
