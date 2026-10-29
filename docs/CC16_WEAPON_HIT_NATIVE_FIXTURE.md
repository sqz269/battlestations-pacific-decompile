# Narrow weapon-accuracy native comparison

This fixture compares the corrected post-spill range/blend window against
original machine instructions. It is not a whole-`008383D0`, size-interpolation,
original-CRT, live-startup, game or ON acceptance claim. The production guard
remains false. Source candidate: `9434d4a37f2a3c4528d826968460a982a4213e53`.

## Reference and exact dependency closure

The installed PE must match SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
No original executable is loaded or launched. The fixture reads only the
following original instruction slices into its private executable allocation:

| Original interval | Bytes | Private offset | Purpose |
| --- | ---: | ---: | --- |
| `00838402..00838526` | 293 | 0 | Post-`008383FE` float-spill window. |
| `00BF7420..00BF743B` | 28 | 304 | Original mode-selected conversion instructions. |
| `00BF7456..00BF74CA` | 117 | 336 | Original x87 fallback instructions. |

The464-byte packed buffer includes26 private INT3 padding bytes. Its unmodified
SHA-256 is `95684f4f4b5bfbec18a19e78354dbb397d8e48bfe9e24e232f2fd5a72c538c6e`.
The32-byte constant buffer contains exact original values from `00D7A24C`,
`00D7A3A0`, `00D0A198` and `00CE3DC0`, plus four private padding bytes; SHA-256
`3fef94215ebdf41133b4da57c3fb4f1272eb92bf642a978ccf2538ee26993dfb`.

Before execution, a full Capstone decode covered all135 instructions. Fifteen
internal direct branches retain their original bytes/targets. There are no
indirect calls, IAT dependencies, unclosed callbacks or unknown memory operands.
Only eight operand fields change:

| Packed operand offset | Width | Exact relocation |
| ---: | ---: | --- |
| 8 | 4 | Immutable float1 to private constant+0. |
| 47 | 4 | Threshold double to private constant+8. |
| 99 | 4 | Double-10 to private constant+16. |
| 106 | 4 | `0083846B CALL 00BF7420`, rebased to private code+304. |
| 133 | 4 | Double+10 to private constant+24. |
| 276 | 4 | Tail-clamp float1 to private constant+0. |
| 306 | 4 | Original `0109EEA4` operand to a controlled synthetic mutable DWORD. |
| 312 | 1 | `00BF7427` short zero-branch to private code+336. |

Every patch checks its original operand bytes. The packed hash pins every
other instruction byte, and runtime checks reject any edit outside those
29 allowed operand bytes. Code becomes RX, constants become RO, and their
sealed postimages are checked after execution phases and at completion.
The synthetic mode cell and profile are checked for unexpected writes.
These private relocated inputs are not the production canonical NativeData
cell and do not prove its initialization or handoff at runtime.

## Frame boundary and source comparison

The reference starts after the native size-weight FSTP. A four-instruction,
ABI-only tail adapter performs SUB ESP,8; PUSH ESI; MOV ESI,EDX; JMP ECX.
This constructs the exact saved-ESI/two-local/return/weight/fraction frame.
The original window performs its own POP ESI, ADD ESP,8 and RET8. No function
is replaced with a returning stub or a C++ numerical oracle.

The linked production library's new explicit-cell API is called with sizes100
and200 and target length100. This chosen finite endpoint yields the stored
size weight1. The reference receives that window input explicitly. Existing
size interpolation is not executed by the reference and is not newly
qualified at PC53 or for arbitrary inputs by these results. There are no
claims about the excluded prefix `008383D0..008383FE` or its `00419010` helper.

The profile uses alternating exact binary fractions to distinguish the
finite conversion boundary. Two input words are used: `3F333333` (float0.7)
and `7FC12345` (quiet NaN). Each is tested at mode0/1 and masked nearest
CW007F/027F, making eight paired window cases. The same inputs also compare
the unspilled multiply-to-conversion boundary directly against the existing
qualified converter, checking low EAX, the three retained lower x87 values,
and the scoped FP observations. This makes both mode branches observable.

The comparison checks result bits, x87 control word, exception-status bits,
TOP and complete MXCSR. It does not compare C0/C2/C3, FIP/FDP, arbitrary GPR
state, errno/matherr, unmasked exception delivery or a binary ABI replacement.
Each call restores its own child thread's saved FP state. The fixture changes
no machine-wide FP or hardware configuration.

## Execution and observed results

The standalone safe-name executable links the current `bsp_core.lib`, using
MSVC Win32 `/W4 /WX /fp:strict /O2 /Oy- /MD` and `/MANIFEST:EMBED`.
Extracted manifest: `asInvoker`, `uiAccess=false`. A parent launches only its
own hidden child, enforces a15-second wait, and can terminate only that owned
child handle. There was one probe run: child exit0, empty stderr, no timeout.
No game was launched and no install/save/profile/WINS state changed.

All8 window pairs and8 direct conversion pairs passed. The legacy API is a
negative control and differs in6/8 cases:

| Input/control | Original and corrected | Legacy |
| --- | --- | --- |
| float0.7, PC24, either mode | `3E800000` | `3E800000` |
| float0.7, PC53, either mode | `3E800002` | `3E800000` |
| quiet NaN, either control/mode | `7FC12345` | `3E800000` |

Direct conversion low EAX is `FFFFFFF9` for float0.7 at PC24 and `FFFFFFFA`
at PC53. For quiet NaN it is `00000000` in mode0 and `80000000` in mode1.
This confirms discrimination of ambient multiplication, unordered routing
and the actual synthetic cell's zero/nonzero branch. Controls were restored;
code/constants/profile/cell postimages were unchanged.
The live original-call checker passed the one copied-window call row and its
Markdown claim with zero failures, unchecked claims or warnings.

The report retains all eight result rows and artifact hashes. Ignored run
artifacts are under `local/weapon_hit_native_fixture/`. The already-recorded
old converter fixtures could not be recovered at their recorded paths; this
one-file check reuses the committed ring fixture's private-payload/hash/FP
patterns without adding a framework. Root owns optional CMake registration,
acceptance of this narrow proof, and all actual OFF/ON game gates.
