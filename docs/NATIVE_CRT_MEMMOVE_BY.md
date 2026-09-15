# Native CRT memmove and vector provider frontier BY

Discovery from published `06e8986477baed33aa46e05c368f65026f4c6df5`.
No source, settings, Ghidra definitions/flow/listing, build, tests, execution,
or push were performed. Preserve `_memmove`, `__VEC_memcpy`, and
`__get_sse2_info`; the unnamed vector engine and physical feature setter are
not assigned invented library names.

| Entry | Complete physical span | Executable bytes / instructions | Other bytes |
|---|---|---|---|
| BF87E0 `_memmove` | 869 B, through BF8B44 | 711 B / 247 | six jump tables 120 B; padding 38 B |
| C0C82B `__VEC_memcpy` | 227 B, through C0C90D | 227 B / 94 | none |
| C0C7A4 vector engine | 135 B, through C0C82A | 129 B / 35 | skipped padding 6 B |
| C27B1C `__get_sse2_info` | 96 B, through C27B7B | 96 B / 44 | none |
| C27B7C physical feature setter | 13 B, through C27B88 | 13 B / 4 | currently no Ghidra function |
| C27ACC guarded SSE2 probe | 80 B, through C27B1B | 80 B / 25 physical | listing contains 45 B / 11 instructions; filter/handler omitted |

All six physical spans equal fresh saved-program bytes and the installed PE.
Registered executable instruction starts are checked against the complete
live listings; the setter has separate raw instruction/boundary proof. Total
complete physical bytes are 1,420, comprising 1,256 executable bytes, 120 table
bytes and 44 padding bytes. The earlier approved 128-byte C27B40..C27BBF window
is retained as partial context, including 55 bytes of the next unrelated
function; it is not counted as another complete function. No rejected
misaligned decode was promoted into evidence.

Six listed direct/tail rows are checked with `verify_report_calls.py`.
The setter's direct call is separately qualified because no current Ghidra
function contains it. Sixteen indirect transfers in BF87E0 are internal table
jumps, with all 30 used table targets checked against executable starts. There
are no imported API calls in these six bodies. The actual SEH4 prolog/epilog
and owning exception/frame domain are reused from BR, rather than expanded.

## BF87E0 original ABI and direction

Cdecl arguments are destination `[entry ESP+4]`, source `[+8]`, byte count
`[+C]`; normal EAX is the original destination, with plain RET/caller cleanup.
EBP/ESI/EDI are saved, EBX is untouched, and ECX/EDX/status flags are volatile.
Arithmetic and comparisons use the exact 32-bit pointer representations.

The routine computes wrapped `source+count`. It takes the backward path only
when destination is unsigned-greater than source and unsigned-less than that
end. Otherwise it uses the forward path. Valid-buffer/nonwrapping native
preconditions remain required for an ordinary overlap-safe contract; there
is no overflow or null validation. Zero count returns the original destination
without a source/destination data access. Equal pointers are not a separate
short-circuit and may traverse the scalar or vector path.

On the forward path only, count >=100h, current DWORD `0109EEA4 != 0`, and equal
low-four-bit pointer alignment permit vector dispatch. The alignment probe
pushes/pops EDI and ESI. At BF881F..BF8821 the original saved ESI/EDI/EBP are
restored, then BF8822 tail-jumps to C0C82B with the original cdecl argument
frame and return address. It does not synthesize a call or pass register
parameters to the vector dispatcher.

Scalar forward copying aligns the destination with one to three byte moves
where count permits, then uses an unrolled 0..7-DWORD path or REP MOVSD for
at least eight DWORDs, followed by 0..3 byte moves. Scalar backward copying
starts at source/destination +count-4, aligns the destination from its high
end, then copies decreasing DWORD addresses and high-to-low remainder bytes.
Its four bulk branches execute STD / REP MOVSD / CLD. Smaller unrolled paths
do not touch DF. Forward REP paths do not first clear DF; the native CRT
requires entry DF=0. Given that precondition, ordinary returns have DF=0.
Faults during backward REP may occur before CLD and after partial writes;
no local SEH/catch/rollback is installed. General EFLAGS preservation is not
claimed.

## Jump-table and padding boundaries

| Physical table | Entries | Use |
|---|---:|---|
| BF886C | 3 | forward destination alignment indices 1..3, logical base BF8868 |
| BF88E8 | 8 | forward DWORD count 0..7 |
| BF8954 | 4 | forward byte remainder 0..3; short path indexes logical BF8964 with count-4 |
| BF89F8 | 3 | backward destination alignment indices 1..3, logical base BF89F4 |
| BF8A84 | 8 | backward negated DWORD count -7..0, logical base BF8AA0 |
| BF8AF0 | 4 | backward byte remainder 0..3 |

The unused index-zero locations of the alignment tables overlap the end of
preceding instructions/padding. They are not additional valid pointer entries.
The two backward/short paths deliberately use negative scaled indices.
Physical tables total 120 bytes/30 entries. The remaining unlisted bytes
are skipped/follow-return NOP forms (`90`, self-MOV or self-LEA), individually
retained and separated from data. No executable listing repair is indicated
by these holes.

## C0C82B vector dispatcher contract

Cdecl takes the same three stack arguments, returns original destination in
EAX, and preserves EBP/EBX/ESI/EDI through a 1Ch local frame. It does not align
ESP to 16 bytes; native stack accesses are DWORD-based. There is no local
feature probe or backward-overlap decision. Entry DF=0 and suitable CPU/OS
SSE2 support are caller requirements for paths using those instructions.

Pointer alignment here is computed as **signed 32-bit remainder modulo 16**
with CDQ/XOR/SUB/AND/XOR/SUB, not simply an unsigned low-bit mask:

- Both signed remainders zero: take `remainder=count&7Fh`. If count differs,
  call C0C7A4 at C0C878 with aligned pointers and count-remainder (a positive
  128-byte multiple), then copy any remainder with REP MOVSB at the tail.
- Equal nonzero signed remainders: copy `16-remainder` leading bytes with
  REP MOVSB, then recursively call C0C82B at C0C8DA with advanced pointers and
  reduced count. For positive pointer representations the prefix is 1..15;
  for negative ones it is 17..31. The recursion reaches aligned pointers.
- Different signed remainders: copy forward with REP MOVSD then REP MOVSB.
  Equal low-bit alignment with differing pointer signs can reach this case.

BF87E0's >=256-byte gate makes its equal-remainder prefix safe under the
valid-buffer contract. C0C82B is not independently a general short/zero-size
memmove: direct equal-misaligned calls can copy a prefix larger than the
supplied count and wrap the remainder. Aligned zero count itself skips both
the engine and tail. No new checks, changed prefix rule, or unsigned-modulo
repair is justified by the original body.

## C0C7A4 complete source-ready leaf

This cdecl engine reads destination/source/count from stack, preserves
EBP/ESI/EDI, leaves EBX/EDX/EAX untouched, and uses ECX=count>>7. It jumps to the
loop unconditionally. Each iteration loads four aligned MOVDQA vectors at
source+0..30h, stores them to destination+0..30h, then loads and stores four
vectors at +40h..70h. It advances both pointers by 80h, decrements ECX and
repeats. XMM0..XMM7 are clobbered; final ECX is zero on valid normal completion.
Arithmetic flags follow the original SHR/DEC schedule; DF is untouched.

There is no API, global, allocation, callback, exception frame, feature check,
stack SIMD spill, prefetch, non-temporal store, or fence in this body. Its real
preconditions are SSE2 availability in the actual execution environment,
16-byte aligned valid source/destination, positive 128-byte-multiple count,
and forward-safe copy direction supplied by the caller. A zero/sub-128 direct
call still executes the first 128-byte iteration and underflows the loop count;
a general safe-copy wrapper would change its contract. An aligned-load fault
or later write fault propagates after any previous accesses/stores.

The complete engine is the smallest ready source packet, preferably exact
naked x86 instructions with the skipped six-byte padding and loop target
explicitly accounted for. Its absence of external providers permits a
qualified leaf implementation without inventing CPU/global owners. This does
not close the dispatcher or `_memmove` at original runtime addresses.

## Actual feature word and remaining owner

The saved PE/Ghidra `0109EEA4` word is virtual-section zero-fill; this is not
runtime capability evidence. The sole indexed write is C27B81. The complete
physical setter at C27B7C calls C27B1C, writes returned EAX unchanged to the
actual word, executes XOR EAX,EAX, and returns. Its only indexed incoming
reference is the real data word CE3700, confirmed to contain C27B7C. The
startup table walker and execution order are not recovered here. Current
function absence and unknown flow properties remain unchanged.

C27B1C saves EBX twice, initializes its local feature words to zero, and tries
to change EFLAGS.ID (bit21). If the observed flags do not change, it skips
CPUID and uses the initialized-zero feature DWORD. On success it restores the
saved flags with PUSH ECX/POPFD, executes CPUID leaves 0 and 1, and records
results. It does not test leaf0's maximum before executing leaf1. It requires
leaf1 EDX bit26 (04000000h), then calls actual C27ACC and requires its nonzero
return, finally normalizing EAX to 0 or 1. The complete routine preserves EBX
and EBP, uses volatile EAX/ECX/EDX, and does not promise EFLAGS preservation.

C27ACC's complete 80-byte physical body is an actual guarded SSE2 instruction
probe. It calls `__SEH_prolog4` with local size 0Ch and actual scope E03A48,
arms state 0, executes **MOVAPD XMM0,XMM1**, writes result 1, then disarms to
-2, reads the result, calls `__SEH_epilog4`, and returns. It overwrites XMM0;
it does not initialize XMM1 or replace the instruction with a CPU-query API.

The current 11-instruction/45-byte listing omits 35 contiguous bytes at
C27AE9..C27B0B. The 28-byte filter loads `[EBP-14h]`, dereferences to the
exception record, then reads the code. It returns 1 only for C0000005 (access
violation) or C000001D (illegal instruction), and 0 otherwise. The seven-byte
handler at C27B05 restores ESP from `[EBP-18h]`, read-clears the result at
`[EBP-1Ch]`, then falls through to disarm/load/epilog. Actual exception delivery
and disposition semantics belong to the original SEH4 dispatcher. No handler
invocation, catch simulation, flow change, or missing-code repair occurred.

The 28-byte scope contains `FFFFFFFE,0,FFFFFFD4,0` as its cookie header and
`FFFFFFFE,C27AE9,C27B05` as its state-0 record: no GS check, EH cookie at
EBP-2Ch, enclosing state -2, and the exact filter/handler targets. BR's complete
prolog/epilog/handler evidence is reused and all 75 frozen artifacts were
checked twice. Correct frame/cookie/FS ownership and the startup table walker
remain required; an arbitrary C++ try/catch or a synthetic hardware-feature
flag does not establish that ownership. No IsProcessorFeaturePresent
substitution is treated as native proof.

Current `game_hosts.cpp`, `game_hosts_frontend.cpp`, and
`game_hosts_init_tail.cpp` actually query
`IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE)` for projected
consumer state; their source bytes are retained. Other current interfaces
borrow or project the flag. Indexed provider lookups and bounded exact-name/
address source searches found no reconstructed implementation of these
memmove/vector/detector entries. Existing BF7680 `_memcpy` policy permits
qualified overlap-safe host copies and limited small-copy evidence; that is
a comparison only, not proof of BF87E0's vector dispatch, flags, or CPU owner.

The BW free packet b7fc7f55 and all 69 frozen artifacts were checked twice with
SHA256/SHA512 as provenance for the actual `_memmove` caller. BR's native SEH4 evidence has a separately verified 75-artifact provenance.
BY's complete
local evidence has its own whole-directory two-pass inventory in the report.
Native instruction/body recovery, source readiness, and runtime ownership
remain distinct conclusions.
