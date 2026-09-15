# Particle lifetime and cache listing repairs

Ten functions contained omitted fallthrough after CRT free calls. The live
bytes matched the installed image over every complete intended body before
mutation. The coordinator used the owning worker worktrees and the shared
Ghidra write lock, clearing only call-site flow overrides. No callee no-return
flags were changed. Truncated function bodies were recreated only after that
independent whole-body byte check; previous names/comments were preserved.

| Entry | Full bytes | Restored bytes | Final listing |
| --- | ---: | --- | --- |
| 86A4D0 | 95 | 86A521..529 | 38 instructions, no gaps |
| 86B6C0 | 23 | 86B6D2..D6 | 10 instructions, no gaps |
| 86BB80 | 206 | 86BC09..4D | 68 instructions, no gaps |
| 86BC60 | 30 | 86BC75..77 | 11 instructions, no gaps |
| 86FCF0 | 120 | 86FD29..67 | 37 instructions, no gaps |
| 870000 | 221 | 8700C0..C9 | 73 instructions, no call gaps |
| 871370 | 23 | 871382..386 | 10 instructions, no gaps |
| 871480 | 95 | 8714CC..DE | 26 instructions, no gaps |
| AF5620 | 50 | AF5630..32 | 21 instructions, no gaps |
| AF5850 | 221 | AF58FB..590C | Restored failure cleanup; no call gaps |

The three bytes87005D..5F remain unlisted after an unconditional jump to870060.
They are `8D 49 00` (`LEA ECX,[ECX]`) alignment padding; this is separate from
the restored call fallthrough at8700C0. The stored body now includes its full
return paths.

An additional existing80-byte instruction body86BA10..86BA5F was defined as a
function after matching its complete live/disk bytes. It now contains30
instructions with no gaps; RET0Ch ends at86BA5F. Its cache-key behavior and
native ABI are recorded by the cache worker's reconstruction packet.

`reports/native_particle_flow_repairs_orch4_h8.json` records flow edits and
readback; `reports/native_particle_body_repairs_orch4_h8.json` records function
recreation/definition, previous metadata and whole-body preflights. The project
was saved and affected exports refreshed. Listing repair alone is not source,
ABI, exception-runtime or gameplay validation; the separate source packets
carry those coverage boundaries.
