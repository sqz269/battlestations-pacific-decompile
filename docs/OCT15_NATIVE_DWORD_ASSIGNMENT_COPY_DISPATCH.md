# Native DWORD assignment copy dispatcher

Address: `006974B0`, existing native identity `STL_TagDispatch_006974b0`.

| Entry | Coverage | Original ABI | Source API |
| --- | --- | --- | --- |
| `006974B0` | complete ordinary dispatcher at the accepted rawDWORD Source boundary | ECX owner; incoming EDX ignored; stack first,last,destination; RET0Ch; EAX advanced core output | `copy_native_dword_assignment_uninitialized_dispatch_006974b0` |

The complete 37-byte/14-instruction body matches accepted `00697860`: same
register operations, stack positions, dead metadata, resolved core target and
result propagation. Only the relative transfer encoding differs. The new Source
entry forwards the same five physical inputs to the full accepted dispatcher.
It introduces no header, allocator, count conversion, ownership or iterator type.
Existing Source implementations and earlier qualified fragments are preserved.

The native dispatcher overwrites incoming EDX with destination before using it.
It saves owner, clears only the low byte of that private saved word, and passes
the core four stacked words: destination, owner, duplicate destination bits and
the dead owner-derived tag. Core ECX/EDX receive first/last. The core consumes
destination; the other three words are not owner/allocator services. After core
RET10h the dispatcher restores its saved word and RET0Ch, propagating core EAX.

Actual `00696D40` captures the last endpoint and destination even for an empty
range. While first differs from last, a nonnull destination reads the current
source DWORD immediately before its store. A null destination skips both
accesses, and both cursors still advance four with DWORD wrap. The returned
pointer is the actual advanced output. Progressive overlap, empty ranges and
null-output advancement retain the accepted implementation's behavior; no
memcpy/memmove, snapshot, length calculation or validation replaces it.

There is no local EH frame, owned object, cleanup or allocation. Child access
faults and current Source exception behavior retain their existing boundary.
The Source call does not reproduce native private-stack aliases or ambient
register/frame state.

| Native owner | Verified native instruction | Role |
| --- | --- | --- |
| `006974B0` | `006974CC CALL 00696D40` | owned actual core invocation |
| `00697860` | `0069787C CALL 00696D40` | complete equivalent reference dispatcher |
| `00697BD0` | `00697C99 CALL 006974B0` | existing-capacity suffix; result publishes end |
| `00697BD0` | `00697CE4 CALL 006974B0` | fresh-allocation copy; result publishes end |

Both assignment sites push destination,last,first with ECX receiver. The first
uses its remaining source suffix and current destination end; the second uses
the current source range and fresh destination begin. Neither changes the
dispatcher contract. These are bounded producer proofs; assignment itself is
not reconstructed by this packet.

Read-only Ghidra queries verified `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe`. All 37 owned bytes/14 starts, 37 reference bytes/14
starts and 45 separate producer bytes/18 starts match disk and live memory:
119 bytes/46 current starts, with four verified CALL sites. The JSON report
freezes the full byte pair, instruction schedule, body ranges, producer bytes,
current transfer ownership and Source hashes.

MSVC Win32 compilation passed, and the three existing suites passed in 7.43
seconds. The actual Release COFF object is x86: its nine code bytes contain
PUSH EBP, MOV EBP,ESP, POP EBP and a tail JMP to the accepted Source symbol.
The sole REL32 relocation is at offset5. The balanced frame restores entry
ESP/EBP; ECX, EDX and all three public stack words remain untouched before the
transfer. Its bytes, symbols and relocation are frozen in the report.
That check establishes the new Source-to-Source forwarding
layout, not original installed-game ABI identity. No new tests or native/game
execution are introduced.

The accepted volatile rawDWORD Source remains qualified for original
private-frame aliases, pointer/fault/SEH behavior, register identity, binary ABI
and gameplay. No generic library model or callback is introduced. Full
`00697DC0` and `00697F40` Source remains conditional on accepted actual
`00697990`; this packet makes no parent completion claim.
