# Native Lua cache string range and vector equality

Addresses: `00B6C480`, `00B6C7B0`, `00B6C990`.

The historical packet/file label `lua_cache_tree` is retained for continuity. These
bodies traverse contiguous vectors of native strings, with no tree operations.
The implementation uses actual headers and preserves the original comparison and
validation schedule. It does not substitute `std::vector<std::string>` equality.

| Native body | Source function | Coverage | Original ABI |
| --- | --- | --- | --- |
| `00B6C480..00B6C4DF` | `mismatch_native_string_range_00b6c480` | complete | cdecl, six physical DWORD arguments, plain RET, EAX=output |
| `00B6C7B0..00B6C815` | `equal_native_string_range_00b6c7b0` | complete | cdecl, six physical DWORD arguments, plain RET, AL Boolean; upper EAX unspecified |
| `00B6C990..00B6CA25` | `equal_native_string_vectors_00b6c990` | complete | ECX=left owner, EDX=right owner, plain RET, EAX=0/1 |

Source: `include/bsp/lua_cache_tree_helpers.hpp` and
`src/lua_cache_tree_helpers.cpp`, registered in `cmake/startup.cmake` for `bsp_core`.
The descriptive names are hypotheses. The established library name
`STL_NativeStringRange_Mismatch_00b6c480` is preserved.

## Layout and ownership evidence

`00432090` is a producer: it clears owner+4/+8/+0Ch, allocates count*8 bytes,
publishes begin/end at +4/+8, then capacity end at +0Ch. `00506C80` consumes that
same representation when assigning the cache names; it preserves owner+0. No
meaning is assigned to that opaque first word. The existing
`NativeLuaScriptCacheStorage` in `bsp/cc13_seg87.hpp` has the same name fields.

The existing `NativeString` declaration and constructor `0041E870` establish
the eight-byte element: recorded DWORD length at +0 and character pointer at +4.
The existing `NativeStringVectorStorage` is a different 0Ch data/count/capacity
container and is deliberately not used here. These three routines borrow all
headers/data. They allocate, free, retain and transfer nothing. The mismatch
routine alone writes two result words, after traversing its inputs.

## Physical arguments and stack cleanup

| Entry ESP offset | `00B6C480` | `00B6C7B0` |
| --- | --- | --- |
| +04h | output, consumed | first, consumed |
| +08h | first, consumed | last, consumed |
| +0Ch | last, consumed | second owner, consumed |
| +10h | second first, consumed | second first, consumed |
| +14h | unused iterator word | unused last copy |
| +18h | unused dispatch word | unused dispatch word |

Six pushed argument words are confirmed by stack cleanup at `00B6C805`,
`00B6CA09` and `00B6C69C`: each instruction is `ADD ESP,18h`. The other
mismatch caller is `00B6C670`. Its unused fifth word is last,
whereas `00B6C7B0` passes first; this does not affect the four consumed inputs.

`00B6C7B0` uses its own incoming third/fourth argument slots as the mismatch
output (`LEA EAX,[ESP+30h]` at `00B6C7FB` after five pushes), and clears the
low byte of its captured last argument's slot before using that word as the
unused sixth child argument. Its held EDI last pointer remains intact. Source
uses a private two-DWORD output and passes captured last with its low byte
cleared, avoiding a C++ write across separate formal arguments.

`00B6C990` passes captured left end twice, captured right owner/begin, captured
left begin, and a dead dispatch word whose low byte is zero. Its high bytes are
indeterminate native local storage. Source passes zero for the entire ignored
word. All six physical words are explicit in the source range/mismatch APIs;
caller-frame aliasing, dead high bytes and machine stack/register identity are
outside the source contract.

## Comparison and validation schedule

`00B6C480` reads both recorded lengths. Unequal lengths stop immediately;
equal zero lengths advance without reading either data pointer. Equal nonzero
lengths compare NUL-terminated text through `00BF7FBF`, so matching embedded-NUL
prefixes may compare equal despite later bytes. It advances both pointers by
eight, stops at the left end or first mismatch, then writes the right result
to output+4 before writing the left result to output+0. Output aliases into
the traversed storage retain that final write order.

`00B6C7B0` captures arithmetic-SAR3 of the wrapping left byte difference before
reporting a null second owner. It then computes the wrapping second endpoint
and compares that endpoint against the CURRENT second end, then CURRENT second
begin (short circuit). It reports failure once and continues without retry.
It does not validate the first range, the second start separately, capacity,
or eight-byte alignment. It tests whether the mismatch left result equals its
captured last pointer.

`00B6C990` computes each count as zero for a null begin, otherwise arithmetic
SAR3(end-begin), using DWORD wrap. Unequal counts return zero before validation.
For equal counts it checks the captured right begin against current right end;
captures left end and checks current left begin against it; captures left begin
and separately checks it against current left end. Every returning invalid
report continues. The captured right begin, left end and later left begin
survive those reports; source preserves the same reload/capture points rather
than normalizing the vector or merging the two left checks.

## Call and library boundaries

| Containing function | Call site | Native callee | Established behavior |
| --- | --- | --- | --- |
| `00B6C480` | `00B6C4B8` | `00BF7FBF` | VS2005 `_stricmp`, locale-dependent, cdecl two arguments; ADD ESP,8 at `00B6C4BD` |
| `00B6C7B0` | `00B6C7C9`, `00B6C7E0` | `00BF6713` | returning invalid-parameter report boundary |
| `00B6C7B0` | `00B6C800` | `00B6C480` | exact native-string mismatch body above |
| `00B6C990` | `00B6C9C9`, `00B6C9DA`, `00B6C9E8` | `00BF6713` | returning invalid-parameter report boundary |
| `00B6C990` | `00B6CA04` | `00B6C7B0` | checked second-range equality above |

The complete listings of both mismatch callers (`00B6C670`, `00B6C7B0`) and
both vector-equality callers were inspected. The setup in `00B6D3C0` is
ECX=input names, EDX=`0108FF30`; the setup in `00B6D400` is ECX=input names,
EDX=cache. Both branch on AL before clearing/assigning/building
the cache. Existing `LuaScriptCacheServices` remains unchanged: no production
host binding to these new source functions is added by this packet.

Exact call receipts from those complete function listings:

```text
00B6C4B8 CALL 00BF7FBF
00B6C7C9 CALL 00BF6713
00B6C7E0 CALL 00BF6713
00B6C800 CALL 00B6C480
00B6C9C9 CALL 00BF6713
00B6C9DA CALL 00BF6713
00B6C9E8 CALL 00BF6713
00B6CA04 CALL 00B6C7B0
00B6C697 CALL 00B6C480
00B6D3C8 CALL 00B6C990
00B6D40C CALL 00B6C990
```

The `_stricmp` native body was read live: its zero-locale-state path uses
`___ascii_stricmp`; its other path uses `_stricmp_l`. Source calls the current
CRT `_stricmp`, preserving the library boundary but not claiming VS2005 locale,
invalid-input/error-state, fault or handler identity. Invalid iterator reports
use existing `invoke_native_invalid_parameter_00bf6713`; its header documents
the current SDK policy and missing native encoded-handler/Watson behavior.

The three owned bodies have no local EH frame or owned cleanup. Source adds no
exception conversion, rollback, `noexcept` or `noreturn` promise. Provider C++
exceptions may propagate. All taken raw reads/writes require valid backing;
source does not emulate native access faults or arbitrary handler aliases into
private native stack frames.

## Verification and limits

Every live query uses `bsp.py`'s verified client for project `bsp` configured as
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, language and image
base. Ghidra was read only; primary integration handles annotations and exports.
Full body pseudocode and assembly were read separately for all three routines;
all known direct caller argument setups and the relevant producers were read.

`./scripts/build.ps1` passed for MSVC Win32 Release, including the existing
`reconstructed_math`, `native_math_differential` and `tool_tests` suites.
The JSON report and this document each pass 11 exact call-site checks with no
failures. Results and the local build log path are recorded in
`reports/oct15_lua_cache_tree.json`. No new routine tests are added.
Existing seed differential checks cover existing
math seeds, not these functions. Source reconstruction/build evidence does not
establish native differential equality, original ABI/EH compatibility, runtime
cache binding, startup execution or original-game/gameplay correctness.
