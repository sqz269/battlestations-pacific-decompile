# Native DWORD uninitialized fill

Addresses: `006970D0`, `00697760`.
Source: `src/native_dword_uninitialized_fill.cpp`.
Header: `include/bsp/native_dword_uninitialized_fill.hpp`.
Report: `reports/oct15_native_dword_uninitialized_fill.json`.

| Native routine | Coverage | Source function |
| --- | --- | --- |
| `STL_UninitFillN_006970d0` | complete scalar loop | `fill_native_dword_uninitialized_006970d0` |
| `STL_TagDispatch_00697760` | complete dispatch and independent result | `fill_native_dword_uninitialized_n_dispatch_00697760` |

The established STL names are retained as analysis names, not recovered symbols.
These operations fill actual four-byte scalar storage. They allocate nothing,
construct no owning object, invoke no allocator or diagnostic, and have no native
owner-unwind action. No generic vector or modern fill service is introduced.

## Native inputs and ordered accesses

The core receives destination in ECX and unsigned count in EDX. Its four physical
stack words at entry are value pointer (`ESP+4`), owner (`+8`), duplicate value
pointer (`+C`), and tag (`+10`). `RET 10h` removes all four. Only the value-pointer
slot is read. A zero count branches directly to the return before that read;
there is no semantic return value. Nonzero execution captures the value ADDRESS
in EAX, then repeats:

1. If the current destination is nonzero, read the current DWORD at the captured
   value address, then write it to the destination.
2. Decrement the unsigned count and advance destination by four with DWORD wrap.
3. Repeat while the remaining count is nonzero.

The null-destination branch skips both memory accesses, and still advances.
The value is reread on every nonnull iteration. An initial scalar snapshot would
change progressive aliases. Neither invalid pointers nor overlap are rejected.
The native EAX register is ambient for count0 and holds the captured value
pointer after nonzero execution; the Source core deliberately returns `void`.

The dispatcher receives ECX owner, unconsumed EDX, and stacked destination,
count, value pointer. `RET 0Ch` removes those three words. It saves the original
destination/count, builds the core's four physical stack slots, and calls it.
Its saved ECX word has only its low byte cleared for the dead tag: upper24 bits
come from the incoming owner, rather than an invented zero-initialized object.
The duplicate value pointer and owner are also dead in the core. After return,
`LEA EAX,[EDI+ESI*4]` computes the result independently from the captured original
destination/count. The core's ambient EAX is ignored.

## Current body and producer evidence

The saved `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, was
verified before/after the read-only native batches. Current typed metadata gives
one complete range per owned routine: `[006970D0,006970F6)` is38 bytes/15
instructions; `[00697760,00697792)` is50 bytes/21 instructions. All88 bytes equal
the installed PE image. All36 current instruction starts, saved listing starts,
and disk-decoded starts agree. There are no external tail jumps or indirect
calls. SHA256 values and exact transfer bytes are retained in the report.

| Site | Actual containing function | Target | Consumed producer contract |
| --- | --- | --- | --- |
| `00697784` | `00697760` | `006970D0` | destination/count captured, four stack words, independent wrapped result |
| `00697F89` | `00697F40` | `006970D0` | actual DWORD Append spare-capacity path; count1, captured current end, then end+4 publication |
| `00697A8A` | `00697990` | `00697760` | growth path; output of prior copy, insertion count, address of retained scalar value |
| `00697B4B` | `00697990` | `00697760` | short-tail path; current end and count minus arithmetic tail distance, same retained scalar address |

```text
00697784 CALL 006970D0
00697F89 CALL 006970D0
00697A8A CALL 00697760
00697B4B CALL 00697760
```

The core has exactly two current CALL xrefs; the dispatcher has two. All four
sites have current instruction, containing-body, default/effective target and
disk/live-byte proof. The Append producer uses `SAR 2` and publishes end+4. The
InsertCount prologue captures the pointed DWORD into its by-value stack word
before its size checks; the two dispatcher sites pass that word's address.
Those contracts establish DWORD storage without inferring a container type.

The cached graph wrongly attributed a site to the neighboring21-byte catch
body. Current xrefs and complete typed bodies give the ownership shown above.
The second normal body range begins at `00697B06`, immediately after the
catch range `[00697AF1,00697B06)`. This packet records the metadata discrepancy
without changing Ghidra or claiming that catch's Source.

## Source boundaries and validation

The Source functions expose all physical argument positions with `__fastcall`,
and the dead words remain explicitly unused. The dispatcher reproduces the dead
tag bit pattern but attaches no ownership or lifetime to it. The loop uses
volatile DWORD loads/stores on the required MSVC Win32 target to preserve each
progressive read/store, and integer addresses/counts use unsigned32-bit wrap.
These are explicit C++ interfaces: matching register/stack shapes do not prove
original code generation, private-stack aliases, ambient register values,
portable pointer semantics, original CRT/fault/SEH behavior or binary ABI
compatibility. Neither routine catches an exception or introduces cleanup.

The historical `append_input_checked_word_storage` implementation and its
`native_input_configuration_modifiers` native fill evidence remain unchanged.
The older checked-storage and settings-vector fragments keep their original
coverage. Full actual parents `00697F40` and `00697990` remain separately
incomplete: closing these fill leaves does not supply the general insertion,
copy, allocator or length-error subtree.

`scripts/build.ps1` passed the Win32 Release build and all three existing suites
(7.83 seconds total). The report and Markdown transfer assertions and
`git diff --check` are also checked; exact counts and frozen reviewed
Source/header hashes are recorded in the report. No new tests, original-native
execution or gameplay validation are claimed.
