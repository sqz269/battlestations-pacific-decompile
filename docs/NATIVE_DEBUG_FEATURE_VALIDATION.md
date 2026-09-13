# Debug-feature owner original/source comparison

One focused, ad hoc Win32 fixture compares normal paths through **eight copied
original bodies / 964 bytes** with the source interfaces in
`native_debug_feature_owner.cpp`. It produced **49 observations / 2,449
normalized DWORD values**, including **16 string allocations and 16 releases**.
The source output equals the original output captured and frozen before the
first source-mode execution. This adds no reconstructed body or permanent test.

The [report](../reports/native_debug_feature_validation.json) records every
span/hash, 34 private relocations, 24 mechanically checked direct call sites,
two explicit Win32 imports, normalization rules, build evidence and scope.
The source reconstruction is documented in
[NATIVE_DEBUG_FEATURE_OWNER.md](NATIVE_DEBUG_FEATURE_OWNER.md).

## Observed behavior

| Original body | Compared normal behavior |
| --- | --- |
| 0051F460, 189 bytes | Fast publication return without manager access; slow construction/publication/one registration in an existing raw manager; balanced Win32 section with tracked depth initially seven; subsequent fast return. |
| 00BE94F0, 109 bytes | All 34h fields and six untouched padding bytes over patterned storage; captured owner return. |
| 00BE8210, 17 bytes | Unconditional clearing of another nonzero publication and CE3818 base stamp. |
| 00BE8350, 1 byte | No change to owner storage or publication. |
| 00BE8DF0, 308 bytes | Three populated 88h records reserved to capacity five; independent deep strings, complete 80h payloads and forward old-string release; signed negative request clamps to one and leaves sufficient capacity alone. |
| 00BE8F30, 154 bytes | Backward string release observes decremented live counts; retained-capacity growth clears new headers/payloads; resize to zero. |
| 00BE9560, 156 bytes | Full feature-array, owner-string and name-array release order; current count/profile/publication observed at each string release; final publication clear/base stamp. |
| 00BE9600, 30 bytes | Flags-zero full destruction returns the captured owner and retains readable caller-owned storage. |

String pointers are normalized to monotonic allocation identities while their
length, contents and terminators are compared. Backing pointers are normalized
to null/non-null because both paths use the unchanged current CRT allocator.
All 52 owner bytes are compared in patterned construction/destruction cases
after pointer normalization. The slow getter allocates uninitialized storage,
so only its six unwritten padding bytes are excluded from that snapshot.

Original transfers between these eight routines remain transfers between copied
original bodies. External calls bridge to the existing source string/string
vector helpers, raw manager getter and registration, or explicit CRT/Win32
support. Global 0109DB70, import cells and the empty literal are relocated only
inside the private instruction copies. No Ghidra or installed-image mutation
is performed by the fixture. Native EH frames remain present, but the fixture
does not throw through them.

## Replay and retained evidence

Preserve the worker's entire `local/debug_feature_validation/` directory before
removing its worktree. The portable minimum is `replay.py` plus the complete
readonly `basis/` directory, containing 21 files. Run:

```text
python <preserved-bundle>/replay.py --repo <built-candidate> --output <new-directory>
```

The helper links the **exact current candidate owner object** and current
libraries. It verifies every MAP-selected object against a unique current
archive member. `/showIncludes` and `/Zs` capture the linked source include
closure; compiler TLog dependencies are checked before linking and after the
runs. Original mode must still equal the fixed original expectation before
source mode executes. Candidate HEAD, tracked content, current/frozen input
hashes, readonly fixture inputs and helper identity are checked. The resulting
`manifest.json` exposes the standard candidate, input, result, executable and
MAP fields for the primary promotion gate. Freshness remains the primary exact
candidate build's responsibility.

Development replay `attempt02` passed at
`c0dc16473e39e095233a2cbedb14e1c362ca7af2`: **15 linked objects, 318 compiler-observed
headers, 2,589 compiler dependency paths and 351 frozen input pins**. The
development strict Win32 build passed both existing CTests after generating
the local verified seed header. The initial configure ran only the first CTest;
its separate log preserves that distinction.

`compile00` retains the initial successful probe-only compilation. `attempt01`
retains the successful original-only capture, inputs, executable and MAP; its
missing-expectation guard deliberately stopped before source mode. `attempt02`
retains both matching runs. A fresh delivery replay is retained at the final
report commit, with its exact HEAD in the ignored manifest.

## Limits

The shared support implementations and the game string pool are not original
dependencies independently compared here. No failing allocation/registration,
reentrant publication change, original FH3/SEH exception, hardware fault or
native no-op EAX identity is exercised. The payload arrays are disjoint, so
REP MOVSD overlap propagation remains assembly evidence. Flags-one owner free,
heap backing-free identity/order, central D68B94 mixed-owner drain dispatch,
parser startup, game reachability and gameplay remain outside these assertions.
This is a bounded source-interface result, not a drop-in ABI or game result.
