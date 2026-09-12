# Actual input action deadline callback

Addresses: `006965A0`, `004D8CD0`.

`native_input_action_deadlines` supplies the complete wrapper and deadline
schedule within an explicit actual-storage/provider domain. It borrows the game
publication, the actual action-owner context, the actual E18A7C-equivalent map
header and the live double CE65D0. It neither owns those objects nor substitutes
`WorldTickState`, `std::map`, a copied clock or a callback that does nothing.
Existing typed source and reconstruction records remain unchanged.

| Routine | Coverage | Original ABI and body evidence |
| --- | --- | --- |
| 6965A0 callback | Complete, two instructions | No native arguments; MOV ECX,[E188A8], then JMP4D8CD0 at6965A6, length5. Eleven bytes, inclusive6965AA/exclusive6965AB. No Ghidra function start during recovery. |
| 4D8CD0 deadlines | Complete within the raw/provider domain | ECX actual game; incoming EDX is not an argument; no stack arguments or semantic result. Plain RET at4D92AA, length1. Body4D8CD0..4D92AA, exclusive4D92AB, 1,499 bytes, 390 instructions, no gaps. |

The wrapper loads the game publication once and supplies that receiver to the
body. Neither routine tests it for null. The body preserves the captured game
through all sixteen action checks, even when a provider changes E188A8. It calls
the existing concrete raw getter `get_native_input_action_owner_004bec00` before
every check, then reads that returned owner's current array pointer at+4.
The fixed record indices, in native order, are:

`4A 4B 4E 4F 50 51 52 53 54 55 46 47 4C 4D 57 58`

Each index addresses a real30h record. No count, enabled-byte or range check is
present. The current latch byte+28 must be nonzero and its float+24 must compare
ordered-greater than zero. The previous state blocks the edge only when its
latch byte+20 is nonzero and float+1C compares ordered-greater than zero. Thus a
quiet NaN in the current value suppresses the edge; a quiet NaN in the previous
value permits it. This follows the native COMISS/JBE and COMISS/JA branches;
replacing the second test with `previous <= 0` loses the unordered case.

For each edge, the body performs FLD of the captured game's float+64C, FADD of
the live **double** CE65D0, and FSTP to a binary32 local before calling subscript.
CE65D0 currently contains bits3FD99999A0000000, the double promotion of0.4f.
The source borrows that live cell and does not replace it with a decimal double
or a fixed float. After subscript returns, a separate FLD/FSTP stores the saved
deadline through the returned pointer. Changes made by subscript to the game
clock, delay, action publication or later records affect later checks only.

| Dependency | Native call sites | Required contract |
| --- | --- | --- |
| Concrete raw4BEC00 getter | Sixteen sites4D8CD6..4D9249, enumerated in the report | No native arguments; EAX actual24h action owner. Reuse the existing context's genuine current publication and shared lifetime behavior. |
| Raw-map subscript4D6900 | Sixteen sites4D8D28..4D929B, enumerated in the report | ECX same actual12h map header; one stack pointer to signed key; EAX writable float cell; RET4 at4D697D. Search or insert a zero-valued missing element, retaining actual storage and library validation/failure behavior. |

The subscript body was inspected through its complete4D6900..4D697F range. It
performs a signed-key lower-bound walk, copies key/zero-value before the insertion
library call4D3CD0, validates the resulting iterator through BF6713 as necessary,
and returns node+10. That insertion graph remains a recognized library/source
adapter boundary; no original STL implementation or symbol reconstruction is
claimed here. All22 callers of subscript were checked: sixteen in this body,
two in4D92B0 using E18A7C, and four in4D9420/4D9480 using game+5C8. A future
adapter must stay header-parameterized and cannot own a separate shadow map.

Producer evidence establishes the borrowed fields. A93DA0 initializes the
actual24h action owner and its array header; A93940 initializes the real30h
record's latch/value pairs. Game constructor4DDB90 captures ECX in ESI at4DDBAB
and publishes that pointer to E188A8 at4DE105. Mission loading restores that
same game's pointer at4E0754, zeros XMM0 at4E0757 and stores float zero to+64C
at4E0764. Frame scaling4C6E30 accumulates the undilated step into+64C at4C6F29
or4C6F76. This is a different clock from the separately accumulated frame clock.

The static map producer CC9E30 publishes the sentinel at E18A80, initializes
its self-links/nil byte, zeros E18A84 and registers CD9EC0 with CRT atexit. The
separate `native_input_action_deadline_map` packet binds and owns that static
lifetime over the same caller-provided12h header. It does not provide subscript.

Both ways to reach the deadline body are recorded. Game_OnMove4E4A40 retains its
incoming game in ESI, moves ESI to ECX at4E4FE9 and calls4D8CD0 at4E4FEB without
stack arguments. The wrapper uses the E188A8 load and unconditional tail jump.
Game_OnInitOnce4DD5B0 installs6965A0 in F8BBFC at4DD71B; A92C40 captures/tests
that callback at its tail and calls it atA92D0D without semantic arguments.
The missing wrapper is reported as `tail_site`/`callee`, without attributing it
to the unrelated preceding Ghidra function. Workers made no Ghidra mutations.

MSVC Win32 Release and both existing CTests passed after eight native seed
ranges matched. One ignored archive-only probe compared the copied native
11-byte wrapper and1,499-byte body against the new code. All-edge and NaN-mixed
cases matched full raw storage and call order, with16 and8 map calls respectively,
and sixteen native getter calls in each. Controlled map callbacks changed the
game/action publications, action base, original clock and delay, verifying live
reloads, captured receiver identity, rounding before callbacks and stores to
returned cells. A source-only exception case preserved an earlier write while
skipping the failed cell store and the rest of the schedule.

Neither native body has a cleanup frame. Provider exceptions propagate with
prior effects intact; the source adds no cleanup or retry. The fixture's map
provider is controlled evidence, not production insertion. Exact hashes and
CALL receipts are in `reports/native_input_action_deadlines.json`. No map
insertion, SDK/device polling, force, ShowCursor or game execution was performed.
Original callable/FH3/SEH ABI, asynchronous mutation, hardware faults and unmasked
floating exceptions are outside this new C++ source interface.
