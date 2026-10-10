# CC12 allocation statistics: physical startup-action tail

The separately admitted eight-byte probe begins with `POP ECX; RET`. Together
with the retained action's load, push and free call, these two physical
instructions establish ordinary argument cleanup and return to the frame-setting
helper at `00C07B37`, conditional on an ordinary cdecl return from free and valid
saved stack storage. This closes the startup action's physical-tail condition.
It does not establish the OS origin of its parent-frame argument, fault cleanup,
Native FH3 identity, or a Source startup implementation.

Only this document and the accompanying report are changed. The evidence was
completed offline at baseline `eba1f3dccfcfa62caa50c64a4926a7998330a4e5` after
the accepted SEH-prologue review. No additional Native queries, image reads,
Ghidra writes, C++ changes, builds or execution were performed during completion.

## Saved metadata and separate physical evidence

The two retained typed replies both independently validate modification 32.
`Unwind@00c86a30` is non-thunk and has no-return false, with the exact complete
saved AddressSet `[00C86A30,00C86A3C)`, twelve bytes. Completeness here refers to
the saved set, not to a complete physical callable body. At `00C86A3C`, typed
metadata reports no exact or containing instruction, no exact or containing
function, no instruction flow and one undefined byte represented as a
`DataDB`. The separate context-zero reply is `No instruction at address`.

Root separately admitted only the raw probe `[00C86A3C,00C86A44)`, eight bytes.
Its retained original/live values are identical:

```text
59 c3 8b 85 d0 fe ff ff
SHA-256 ada33d4209a2df1bb9a9dc9d2680d04f210d1d19124324bcfb0caaf20573bbd2
```

Retained PE mapping places the probe in `.text`, file offset `00886A3C`.
The original capture recorded stable size/mtime. The earlier whole-image
SHA-256 is `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`;
that old integrity value is retained provenance, not a newly repeated image
check. Completion reads only retained files, never the installed executable.

The original bounded decoder stopped at the first ordinary near return.
Offline replay decodes only the already interpreted two-byte prefix:

| Physical address | Byte | Interpretation |
| --- | --- | --- |
| `00C86A3C` | `59` | `POP ECX` |
| `00C86A3D` | `C3` | `RET` |

The six bytes `[00C86A3E,00C86A44)` remain uninterpreted. Neither table entry
claims a stored Ghidra instruction: `00C86A3D` was not separately queried.
No instruction was created and no function body was extended. The retained
`CALL_RETURN` override at `00C86A37` remains in place; its saved effective
flow is still a call terminator. A physical processor return from free is
separate from that preserved analysis override.

## Ordinary stack derivation

The accepted startup packet retained the twelve-byte sequence
`MOV EAX,[EBP-130h]; PUSH EAX; CALL 00BF65AC`. Nested typed metadata identifies
the returning free thunk and its returning `00BF9DC8` target. Their bodies
were not opened here. Let A be action-entry ESP and P the current word loaded
from `[action EBP-130h]`. The accepted frame-setting helper calls the action
at `00C07B35`; the original action return word is `00C07B37`.

| Point | ESP relative to A | Relevant stack word |
| --- | --- | --- |
| Action entry | `0` | Original return `00C07B37` at `[A]` |
| After `PUSH EAX` | `-4` | Free argument P at `[A-4]` |
| Free entry after CALL | `-8` | Free return `00C86A3C` at `[A-8]` |
| Ordinary cdecl free return | `-4` | Free argument slot at top |
| After physical `POP ECX` | `0` | Original action return at top |
| After physical `RET` | `+4` | Control reaches `00C07B37` |

Thus the action removes its four-byte free argument and consumes its own
return word. The accepted helper's action-entry coordinate is A=`T-20h`,
where T is helper-entry ESP, so control resumes at `00C07B37` with
ESP=`T-1Ch`, ready for the retained `POP EDI; POP ESI` sequence.

The `POP` overwrites ECX with the argument slot's then-current contents. The
proof does not require that value still equal P after free. The two tail
instructions preserve EBP and do not add another cleanup call or publication
store. They do not establish free's internal effects, successful release, or
fault behavior. The original action return word and required saved stack
storage must remain valid. The algebra is offline bookkeeping, not Native
execution, an ABI test or an exception test.

The accepted actual SEH prologue establishes `00C069A2` EBP at its entry ESP
minus four. Combined with the accepted wrapper, dispatcher and frame-setting
helper, action EBP is the forwarded raw second argument plus `0Ch`. The
prologue's own FS node is distinct from that forwarded argument. Substitution
of the parent's registration node remains conditional on the unresolved OS
entry association. If that association holds, the current load addresses the
parent allocation spill; it does not fetch the current `0109CEFC` publication.

## Source and retained evidence closure

The packet freezes 96 full current inputs, including 40 Source files, as both
working bytes and Git blobs. Ninety-five match exactly; one older qualified
report differs only by CRLF/LF. All 92 inputs inherited from the preceding
SEH-prologue packet remain byte-identical to that packet's working snapshots.
Eleven bounded current excerpts, three complete sorted queries and eleven
retained predecessor files, including the immutable preceding evidence ZIP,
accompany the captures. Full snapshots supply provenance without whole-file
semantic credit.

Current Source still offers ordinary raw allocation/free services, passed-storage
base-profile and guard cleanup, and a reader state model with explicit Native
CRT/FH3 limits. The inspected startup path constructs a local semantic
`AllocationStatsState`; its constructor sets a null profile, budget and zero
startup allocation count. The exact tail/frame-address query has no matches.
The retained publication/profile query and startup-site query preserve current
context. These bounded findings do not prove whole-program absence and do not
supply the actual raw owner, Native action adapter or startup failure ownership.

Two typed raw replies and ten other raw HTTP replies are preserved separately:
the latter comprise nine GETs and one read-only context-zero POST. The POST
only requests exact listing context and returns no instruction. It is not a
Ghidra mutation. Loaded Java CodeSource remains unattested. Earlier epochs,
broader predecessor captures and old execution results do not enter these
current response counts or receive fresh credit.

The read-only offline verifier checks raw response hashes, identity/schema,
epoch, saved metadata, bounded bytes and two-instruction decoding, retained
call operands, stack coordinates, all current/Git snapshots, excerpts, queries,
report pins, prior bundle integrity and every ZIP payload hash/CRC. Bundle
hashes are pinned outside the bundle to avoid self-reference. No GPR save,
annotation, repair, ledger change, build, probe execution, OS dispatch, Native
ABI execution, startup run or gameplay validation is claimed.
