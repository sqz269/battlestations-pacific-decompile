# Raw particle shader and consumed Layer-name helpers

## Scope

| Address | Complete span | Bytes | Original ABI |
|---|---|---:|---|
| B089E0 | B089E0..B08ABB | 220 | ECX Sprite; stack C string; RET4 |
| B06210 | B06210..B062EB | 220 | ECX Axial; stack C string; RET4 |
| B07C80 | B07C80..B07D5B | 220 | ECX Floating; stack C string; RET4 |
| AF4360 | AF4360..AF4449 | 234 | ECX resource; by-value length/data; EAX index; RET8 |

The four complete bodies match installed PE and live Ghidra bytes. Each shader
has 69 instructions, identical after relocating its body/handler addresses.
AF4360 has 86 listed instructions and one three-byte jumped-over alignment gap;
no repair is needed. Existing host interfaces remain unchanged.

## Shader ownership and publication

The raw overloads borrow `NativeStringRawPoolContext&` and use the genuine
resize, constructor, equality and pool return providers. They create an actual
8h comparison header, resize it to eight bytes, capture its data pointer, and
copy the current length plus terminator from the verified `Additive` literal.
Only then is caller state0 armed and the input header constructed.

The caller's sole unwind action destroys the **current comparison header**.
The input header has no caller-owned unwind state. Normal cleanup captures input
data/length and returns it explicitly while state0 remains active. State becomes
-1 before returning the earlier captured comparison-data pointer with its current
length. Definition+7C is written only after both normal releases succeed.
A second exception during the actual state0 cleanup terminates.

The three one-state maps are DF3B1C -> CBB960 (Sprite), DF3954 -> CBB800 (Axial),
and DF3A68 -> CBB8D0 (Floating). Their FuncInfos are DF3B24, DF395C and DF3A70;
all funclets call the genuine 41DD20 destructor. Raw known-target dispatch also
retains the existing genuine Object/Tracer one-byte no-op methods; an unrecognized
target returns false to preserve the caller's foreign-method boundary.

## Consumed Layer-name lookup

The raw AF4360 overload receives the actual 8h by-value argument header. This
explicit source parameter models the original stack object; it is not a binary
ABI replacement. Length is captured once. The function walks actual resource
rows at +34, reloads signed count +54 after each comparison, checks equal lengths,
and compares nonempty names using the current C runtime's case-insensitive call.
The consumed argument data is reloaded for comparisons and the normal pool return.

A match returns its zero-based index; no match also returns zero. The argument's
captured length plus one is used for the return. Its header remains stale and
must not be released again. DF29D8 contains one map entry DF29D0 -> CBAB90,
but the entire body keeps its state at -1; it never stores state0. The raw
companion therefore adds no exception cleanup or retry to that caller.

## Validation and limits

- Strict Win32 `scripts/build.ps1` and all three existing CTests passed.
- An ignored probe copied all four complete original bodies (894 bytes), kept
  internal branches unchanged, and relocated external calls/literal pointers to
  the same genuine source providers and current CRT used by the raw overloads.
- Seventeen original/raw comparisons passed: all three shaders with exact/mixed
  case, missing and empty values; Layer lookup with a final-row match, no match,
  an empty-name match, zero count and negative count. Owner bytes, returned index,
  stale consumed header, and actual pool bump/live/peak counters agree.
- Full body, EH funclet/handler, FuncInfo/map and literal live/PE bytes accompany
  the report. Native FH3 failures, invalid pointers/stack aliases, arbitrary
  reentrancy/concurrency, original CRT identity and gameplay remain unvalidated.

Names are descriptive hypotheses. The raw interfaces close these helpers through
actual string-pool storage; texture/model resource loading remains separate.
