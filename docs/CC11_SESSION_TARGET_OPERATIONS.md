# Session target mask and history operations

Three complete native leaves now operate on the actual borrowed mask and the
existing constructor-owned target/history storage. Names describe inferred
behavior, not recovered symbols. These are new Source interfaces, not binary
replacements for the original calling conventions.

| Body | End-exclusive interval | Ordinary native ABI |
| --- | --- | --- |
| Release selected slot | `[007827D0,007827E6)`; 22 bytes | ECX unsigned index; RET |
| Append signed sample | `[007831E0,00783226)`; 70 bytes | ECX target; signed sample on stack; RET4 |
| Reset history | `[00783230,00783274)`; 68 bytes | ECX target; raw float word on stack; RET4 |

The slot helper uses the actual borrowed `F871B0` word. Its unsigned comparison
skips the mask access for indices above 15; valid indices use the native
unprefixed word AND. There is no lock, new mask, fallback or universal lifetime
ownership assertion. `00773120` is its sole current native caller, and releases
the constructor-selected bit before target destruction.

Both history operations capture the actual target `D54` history once and add
no allocation, profile dispatch, initialization or destruction. Existing array
ownership and all untouched target/history fields remain with the caller.

The append body first captures the index, loads the stored total into x87,
captures the array and subtracts the old sample. Before storing that result,
`CVTSI2SS` converts the original signed integer according to ambient MXCSR.
It stores the x87 difference, then writes the converted sample through the
first captured array/index address. It subsequently reloads the index and
array, loads that freshly selected sample into x87, increments the index and
compares against the freshly observed count. The x87 addition precedes index
publication, and the final total store follows it. Equality from the earlier
comparison controls the final zero-index wrap. These observations are not
folded into a conventional C++ rolling-average expression.

Reset reads and compares signed count before publishing the raw initial float
word. Each entered iteration freshly loads the array, executes `FLD` of the
current initial field and `FSTP` into the sample, then compares against the
fresh signed count. Those conversions matter for denormals, NaNs and FP state;
a word-copy fill would differ. After the loop, the body freshly performs
`FILD count`, publishes index zero, performs `FMUL initial`, and stores the
total. Zero or negative signed count skips the sample loop but still performs
the final arithmetic. The six-byte branch-skipped padding at `0078324A` has no
Source operation.

The implementation retains these instruction schedules explicitly. It does
not normalize x87 or MXCSR, clamp indices, replace signed comparisons, cache
fresh fields, or add guards absent from the native bodies. Callers provide
valid reached storage and keep its owners alive. The current native caller
`007833E0` and its timing globals/conversion remain outside this packet.

An ignored focused probe used two complete Source-constructed targets, each
with its actual owned 50-element D54 history, three-element D58 history,
cursors and lock. Both were explicitly destroyed afterward. The original
history bodies were copied in full with zero relocated bytes. The original
mask body changed only its four-byte absolute global operand to the fixture's
actual borrowed mask cell. All copies were sealed executable after checking
the unchanged bytes; no original CRT or game process was invoked.

Fresh strict Win32 compilation and execution passed 305 checks:

- 99 whole-original append comparisons: 96 combinations of three x87
  precisions, four x87 rounding modes, four independent MXCSR rounding modes,
  and empty/three-occupied x87 stacks, plus zero and signed integer extremes.
  The main samples were positive/negative 16777217, with valid indices 0/49
  under count 50, covering both wrap outcomes and SSE rounding.
- 31 whole-original reset comparisons: the 24 x87 precision/rounding/stack
  combinations plus signed zero, quiet/signaling NaN, minimum subnormal,
  infinity and signed-count edge cases. Mutable helper-level count fixtures
  were -1, 0, 3 and 50 over valid 50-element allocations; only 50 is the D54
  count produced by the constructor. This is not a producer-reachability claim.
- 21 whole-original mask comparisons: actual constructor reservation and
  early release, every valid bit index, and unsigned out-of-range values.
  Adjacent word sentinels remained unchanged.

The history comparisons checked all 50 raw sample words, history fields and
ownership identities, x87 CW/SW/FTW and all eight 80-bit register images,
MXCSR, and all 16 bytes of XMM0. Instruction/data pointer addresses were
excluded. Outer target bytes and the second history's storage were preserved.
FP exceptions were masked, DAZ/FTZ disabled, and an x87 slot was available.
The probe's chosen arithmetic contents do not establish whole timing-state
reachability, concurrent mutation behavior or unmasked fault handling.

The build used six freshly compiled actual translation units, 25 immutable
source/header inputs and a separately hashed probe, with an embedded
`asInvoker` manifest. Three support libraries from the completed main build
at `ae90949a7` were copied at metadata head `f3027c0f4`; original and copied
hashes, input hashes and the probe hash matched before and after linking and
execution. Main was released for its next rebuild only afterward.
Generated assembly was inspected for every captured/fresh access and mixed
x87/SSE ordering. These three leaves contain no native calls; no call-row
verifier pass is substituted for their complete original-body comparisons.

No tracked tests, Ghidra writes, whole-class ABI, derived publication, original
private CRT/EH, network sending or gameplay validation are included. Primary
registration, full main build and integration remain separate work.
See `reports/cc11_session_target_operations.json` and the reproducible ignored
artifacts under `local/cc11_target_operations_20261007_a/`.

## Primary integration

Main `6b06cfd166af854345d0d97fa3488eb53a969398` passed the full Win32 build and all three CTests. Root independently compiled seven actual TUs and checked 26 current Source/header inputs, three current main libraries and the original PE before/after linking. All160 original bytes separately matched live memory. The manifested PE32 fixture passed305checks across151complete original-body cases, including full XMM0 and independent x87/MXCSR rounding. Source-owned count50, valid-storage helper edge counts and whole-timing producer reachability remain distinct. No vacuous call-row check substitutes for whole-body comparison. Current timing/derived/runtime/nativeABI/game qualifications remain.
