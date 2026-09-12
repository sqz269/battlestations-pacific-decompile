# Ship AI avoid-zone query proof

The integrator's 009D7050 query-cache implementation and its two list helpers
passed a bounded original-byte comparison on the represented domain: disjoint
query/cache storage, a nonempty manager table with real ordered groups, valid
native polygon records, and successful matching allocation services. This
packet changes no reconstruction source, address names or Ghidra state.

| Entry | Inclusive body | Native ABI | Reviewed scope |
| --- | --- | --- | --- |
| 009D7050 | 009D7050..009D724E | ECX searcher, query pointer, RET4 | Complete core |
| 00419FA0 | 00419FA0..00419FB9 | ECX list, integer layer, RET4 | Complete list-key helper |
| 004224C0 | 004224C0..004224F2 | ECX list, box pointer, RET4 | Complete refill helper on represented manager domain |

All three complete byte spans match the installed executable and live Ghidra.
The report records final instruction lengths, exact reviewed-source hashes,
supporting body hashes and every native call row. Names are hypotheses; these
are C++ interfaces, not binary replacements.

## Static findings

009D7068 performs an **integer CMP** between cache+14h and query+10h. Keys
must retain all 32 bits: equal NaN-shaped bit patterns compare equal, positive
and negative float-zero-shaped patterns differ, and adjacent large integers
must not be rounded into one float. The new source uses integer equality.

Both query corners must pass actual 00414F50 containment. Lower bounds are
inclusive; upper bounds are exclusive. An upper-corner miss skips the lower
test. Disabled searchers leave the complete cache/list state untouched.

The growth path rounds width/height differences to float, halves them with
the double at D7A280, and expands query half-extents with CEC160. CEC160 bytes
`00 00 00 40 33 33 f3 3f` encode **1.2000000476837158203125**, not double 1.2.
The max keeps the query candidate on unordered comparison; the subsequent
500-double floor also keeps unordered candidates. The replacement 500 value
comes from float CE397C. The fixture includes an expansion-rounding boundary
above the old extent, so the factor is not hidden by the max/floor.

Ghidra renders the FMUL instructions at 009D7119 and 009D7132 as `FMUL ST1`.
Their **DC C9** bytes specify destination ST1 (`ST1 *= ST0`). Together with the
following FXCH/store, they validate the integrator's half/expansion arithmetic;
reading the shorthand as destination ST0 would produce different quantities.

The cache key is stored before either list helper. 00419FA0 clears on integer
list-key change, including an empty head, then stores the list key. 004224C0
clears a nonempty head, obtains the manager, **then reads the live list key**,
selects the actual group and assigns its newly selected list. The fixture's
manager callback sometimes changes the list key at precisely that boundary.

An empty semantic manager table is an explicit representation exclusion.
Native 004120D0 with count<=0 still reads manager slot0; it neither guarantees
null nor inherently faults. The semantic vector does not represent that spare
slot, so the integrator's explicit rejection is appropriate. An existing group
with zero zones is a legitimate represented result and is exercised.

The native min-x store at 009D71F1 precedes the z spill at 009D71F5, but the z
**load** at 009D71EE precedes that min-x store. The integrator corrected the
source comment after the compiled snapshot. An exact comparison verifies that
this is the sole subsequent source change; both hashes are in the report. This proof accepts
the tested disjoint, masked-FP result/callback behavior and does not claim
unmasked exception ordering or instruction-for-instruction FP state identity.

## Two caller contracts

Live xrefs identify two callers, correcting the initial sole-caller description.
Both construct stack queries disjoint from their searcher owners:

- 009DA854 in 009DA6E0..009DA860. ESI receives incoming ECX at 009DA6E4;
  EDI=ESI+A24h at 009DA713 is the searcher. The query begins at ESP+Ch, receives
  owner+184h/+188h coordinates and owner+168h key, and is passed at 009DA83B.
- 009DC3B2 in 009DC2E0..009DCEA2. ESI receives incoming searcher ECX at 009DC2EE;
  its next write is the post-call ADD ESI,18h at 009DC3B7. EDI receives the
  caller's stack input at 009DC2FA. Query ESP+9Ch gets that input's +0/+4
  coordinates and +20h key; both half-extents come from the rounded square root
  of `max(input+10h,input+14h)^2 + (input+18h)^2`. The comparison prefers +14h
  on unordered. The broader input record's semantic meaning is not invented.

Full caller listings and filtered receiver-register writes are preserved in
ignored evidence. Reading these producer portions does not reconstruct either
complete caller in this proof packet.

## Original-byte fixture

One ignored Win32 `/O2 /fp:strict /MANIFEST:EMBED` probe compiles the integrator's
exact source/header snapshots and links a frozen copy of its core library.
It maps original installed bytes privately at 30000000 and adjusts only five
decoded absolute constant operands in the executed code. Native installation
and saved analysis remain unchanged.

The original core and both helpers execute along with eight complete native
support bodies: 00414F50 containment, 004120D0 group selection, 00417A40 group
segment selection, 00417630 zone selection, 00415190 node initialization,
004158A0 list clearing, 004F2B00's box adapter and actual 0085C910 segment math.
Original 00417A40 runs in a complete-body trampoline so a recording wrapper
can observe its arguments without replacing its selection behavior.

Only manager lookup and allocation/free ownership are fixture services. The
manager has three ordered signed keys, including an empty group; its native
pointer slots and semantic groups describe the same fixed polygons. Allocation
returns actual 20h records filled with A5, and free releases matching allocations.
The lazy singleton constructor and CRT allocation-failure/retry paths are not
executed. Initial lists include terminal-marker-bypassed cycles; original
004158A0 performs the observed cleanup.

**1,536 cases passed**: 256 frozen inputs under six x87 controls
027F/037F/067F/0A7F/0E7F/007F, with MXCSR set to 1F80 before each side.
There were **1,290 refreshes, 246 reuses, 8,484 matching callback records and
4,296 matching selected-node records**. Inputs include raw integer-key edges,
half-open boundaries, cache growth/floor/rounding cases, negative/zero extents,
subnormals, infinities, quiet/signaling NaN patterns and deterministic finite
samples. No exhaustive-domain claim is made.

The comparisons cover every cache word including padding, both list words,
all live node bytes and links after pointer-ID normalization, and every observed
allocation/free/manager/selection callback's order, arguments and complete
cache/list snapshot. The new bool diagnostic matches whether native refill ran.
Both complete serialized result streams are **567,744 bytes** and have SHA256
`16869c75e12ace48c48bee3b86e623c5853c1762ae481bf4cf576f3a727afcc7`.
All 256 inputs are frozen as 14 little-endian words each in
`local/query_proof_inputs.bin`; source and manifest specify their fields.

The fixture **does not compare final x87 CW/status/TOP or MXCSR**. Its control
settings are explicit inputs, not a claim of output-register agreement. No
unmasked hardware faults, allocation failures, concurrent geometry mutation,
aliased query/cache, missing raw manager slot, native EH ABI or mission/gameplay
behavior is validated. A focused compiler/link check was run; this docs/report
packet does not claim a separate full-repository build or CTest run.
