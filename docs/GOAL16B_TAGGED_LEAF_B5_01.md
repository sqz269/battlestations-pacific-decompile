# Tagged B5 return leaves

Seven new registered C++ entry bodies reconstruct complete native routines.
Each entire native body is one byte, `C3` (`RET`); each entire strict and
registered Source executable section is three bytes, `C2 00 00` (`RET 0`).
These are complete semantic reconstructions, with no byte-identity claim. These are measured
native no-ops, with no unresolved helper, global, class layout or provider.
The descriptive names are operation hypotheses, not recovered symbols.

| Original address | Previous tag name | Source symbol | Whole Native / Source bytes |
| --- | --- | --- | ---: |
| 00b540a0 | TRIV_body_00b540a0 | bsp_tagged_b5_00b540a0 | 1 / 3 |
| 00b542b0 | TRIV_body_00b542b0 | bsp_tagged_b5_00b542b0 | 1 / 3 |
| 00b544e0 | TRIV_body_00b544e0 | bsp_tagged_b5_00b544e0 | 1 / 3 |
| 00b546e0 | TRIV_body_00b546e0 | bsp_tagged_b5_00b546e0 | 1 / 3 |
| 00b548b0 | TRIV_body_00b548b0 | bsp_tagged_b5_00b548b0 | 1 / 3 |
| 00b5fd50 | TRIV_body_00b5fd50 | bsp_tagged_b5_00b5fd50 | 1 / 3 |
| 00b5fd60 | TRIV_body_00b5fd60 | bsp_tagged_b5_00b5fd60 | 1 / 3 |

OriginalABI is the measured physical body: read the existing return address at
`[ESP]`, increment ESP by four and return. There are no consumed register or
stack arguments, no prescribed result, no callee argument cleanup, no changes
to incoming general registers/EFLAGS/x87 state, and no data accesses beyond the
return stack operation. `extern "C" void __cdecl(void) noexcept` is the new Source
interface. A sole RET cannot recover an original class, formal parameter or
return types, calling convention, or vtable slot. Incoming EAX is preserved,
not assigned a fabricated void-result value. No call-site or game ABI adoption
is claimed. No invalid stack/return-target behavior is separately tested.
RET0 adds an immediate zero to ESP after popping the return address, so it has
the same defined stack/register/flags effects. The longer Source instruction
fetches two additional code bytes; code-fetch fault equivalence is not claimed.

The index-backed inclusive B5 sweep contains 229 function records, including
20 tag-only candidates. This is a metadata count, not a claim that the other
209 entries all have complete Source. Exact-address searches across Source and
headers found no prior representation of the selected seven. The generic
render-FX no-op table has no row for these addresses; initializer/query lifetime
modules do not supply these entry bodies. The selected routines have no named
or unnamed dependencies, so no named-but-incomplete child is promoted to a
complete dependency by this packet.

The thirteen remaining tag-only scalar-deleting destructors are deferred:
`00b51dd0`, `00b56ac0`, `00b56b40`, `00b56d40`, `00b56d60`, `00b56da0`,
`00b57380`, `00b58580`, `00b5b5f0`, `00b5b800`, `00b5e730`, `00b5e7f0`,
and `00b5fd70`. Tags alone establish no complete scalar-delete Source body.
The two sampled closures (`00b51dd0` and `00b5fd70`) call a named destructor and
the real `_free`; these are not independent no-call leaves. Complete conditional
free, return behavior and production lifetime wiring remain outside this packet.
No fake deallocator, shadow global or fallback destructor was introduced.

Read-only live typed metadata verified `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, language `x86:LE:32:default`, image base `00400000`.
Each body has a complete single-address range and a terminal instruction with
no flow override. Every native byte was captured through `bsp.py ghidra bytes`
and compared with the installed PE. No Ghidra annotation or analysis was changed.
An initial local capture rejected a path-spelling comparison before reading
native bytes; the comparison was normalized and rerun. This setup correction is
included in inclusive elapsed time. Initial strict validation also rejected
the incorrect one-byte Source assumption: actual full sections contain RET0.
The strict object also has seven `.debug$F` metadata relocations to its own
function symbols; an initial all-sections-zero-relocations assumption was
rejected and corrected. All executable sections have zero relocations.
The same C++ form was retained and reclassified as complete semantic Source;
there were no compiler failures or rejected Source forms.

Strict MSVC Win32 `/std:c++20 /MD /O2 /W4 /WX /fp:strict /Gy` and the actual
registered Release/Win32 object account for all seven nonempty executable
sections, every symbol offset (zero), all section/data bytes and all relocations.
Executable relocations are zero; metadata relocations are recorded separately.
The complete Source executable total is 21 bytes / seven instructions; the
referenced native bodies total seven bytes. Exact-Native-byte count is zero. There is no
Native-size prefix clipping, tail exclusion, masking or external relocation.
The archive check requires exactly one real `bsp_core.lib` member containing
this module, with content byte-identical to the actual registered object.

The append-only CMake registration was added without claiming the shared file.
The required `scripts/build.ps1` completed its full Win32 build and two configured
CTest checks. No test cases or ad hoc executable probes were added. This is
whole-body code-generation and build/fixture evidence; native execution, game
integration and game validation were not performed.
The fresh build plus CTest interval was 1036.1200392 seconds, ending
2026-10-16T05:41:23.2908389Z; CTest itself took 8.20 seconds. Strict compilation
took 1.8875167 seconds. The build reported one LNK4006 duplicate-definition
warning in existing spawn code (`lua_binding_spawn.obj` / `lua_spawn_new.obj`),
outside the owned module. The report-call checker found zero failed claims;
the two sampled native closure statements have no individual call-site rows
and were reported as one unchecked prose claim, not verified call-site proof.

The tracked JSON report records full sections/symbols, native/source/header/
object/archive hashes, production archive-member offset and size, deferred
coverage and timing. Raw typed metadata, native byte captures, strict compile
log, full build/CTest log, validation/scoping helpers and inclusive final handoff
timing remain under ignored `local/goal16b_tagged_b5/`. Timing begins at
2026-10-16 05:17:50 UTC before brief, worktree creation and index rebuilding;
final commit, lease release and handoff overhead are retained in that final
timing artifact. Seven function identities do not imply a speedup over any
other reconstruction packet.
