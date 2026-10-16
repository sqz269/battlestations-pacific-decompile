# GOAL16Q: complete raw Adler32 entry

This packet implements 00BCD720 `adler32` as a meaningful new uint32 fastcall
interface: seed in ECX, byte address in EDX, length in one actual stack DWORD,
RET4. The correct library name is retained. Original formal types, decoration
and historical compiler flags remain hypotheses; no caller is redirected.

The complete readable literal MSVC32 body is 297 bytes / 110 instructions.
Strict C++17 and actual registered sections equal all Native bytes, including
both returns and padding. No prefix clipping, helper omission, stockcdecl
substitution, fake input/global/provider/class or domain restriction is used.
Literal identity is an available evidence path, not universal Source policy.
Machine-readable evidence: `reports/native_zlib_raw_adler32_bc.json`.

## Entry, conditional frame and edges

PUSH ESI, PUSH EDI precede copying EDX to EDI, zero-extending CX into ESI,
shifting ECX by sixteen, and testing EDI. Null bytes pop EDI, compute EAX=EDX+1
(one), pop ESI and RET4. The actual length slot is never read on this path;
EBX/EBP are never saved. Stack accesses still precede the null test and can
fault. Null-return flags remain TEST-zero; ECX is the original high seed half.

Nonnull bytes push EBX and then read length at current ESP+10h, equivalent to
entry ESP+4. Length zero skips EBP save, all byte reads and all reductions. It
combines the original sixteen-bit halves and returns the full seed unchanged,
including noncanonical seeds such as FFFFFFFFh. EDX remains the input byte
address. No half is normalized before a first positive chunk.

Positive length saves EBP once. Unsigned min(remaining,5552) selects each chunk;
EBX retains the decreasing remaining count. The signed chunk<16 branch is
well-defined for the already-clamped 0..5552 chunk. Sixteen individual increasing
BYTE reads are interleaved with the exact low/high additions, then the scalar
tail reads increasing individual bytes. No widened loads, pair snapshot,
delegated library call or pointer-difference arithmetic replaces that schedule.
ADD16/ADD1 address changes wrap modulo 2^32. Observed LEA ESP,[ESP+0] padding is
explicit 8D642400; the tail NOP is retained.

Each positive chunk reduces low and high separately with unsigned MUL by
80078071h, high32 SHR15, low32 IMUL by -65521, then ADD. Positive remaining counts
decrease until zero; no data-dependent nontermination or reduced length domain
is introduced. Ordinary completion requires every actually reached data, stack
and control cell and an ordinary return path; invalid accesses retain the
statically identical fault-capable instruction schedule.

Nonnull EAX is the checksum; ECX is its high half (original high half for
length zero). On positive paths EDX retains the final negative quotient-times-
65521 value, not a semantic result. Final flags come from OR: CF/OF zero,
SF/ZF/PF from the checksum, AF undefined. DF and all x87/SIMD/FIP state are
unchanged. Actual POP cells determine nonvolatile restoration, with EBP popped
only on the positive path. Normal RET4 advances ESP by eight including return.

Input bytes may alias arguments, saved registers or control/return cells.
Initial PUSH writes can change later byte inputs; POP/RET read actual current
cells. The complete frame/access/register/flag schedule is retained. No faults
were dynamically measured or induced: identical fault-capable instructions and
prefixes are static evidence. Source/code/current-stack placement, aliases at
Original absolute code addresses, fault/return EIP values, Native SEH/runtime/
full ABI, caller adoption, installed replacement and game behavior are unproved.

## Static all-word arithmetic

Let d=65521, m=2147975281, A=2^47, e=m*d-A=31073 and qmax=65551. For every
uint32 x=q*d+r, 0<=r<d, multiplication has residual q*e+r*m. It is nonnegative
and at most qmax*e+(d-1)*m=A+(qmax+1)*e-m<A, since
(qmax+1)*e=2036897296<m. Therefore floor(x*m/2^47)=floor(x/d) for every uint32
input. MUL high32/SHR15 computes that quotient; the low32 negative multiply plus
x yields the exact remainder. No sampled runtime cases or probes are used.

Every seed splits into arbitrary halves <=65535. With n<=5552 and byte<=255,
low<=65535+255*n and high<=65535+n*65535+255*n*(n+1)/2. At 5552 the bounds are
1481295 and 4294773495, below 2^32; subsequent reduced halves<=65520 only lower
them. These monotone bounds cover all seed, byte and uint32 length patterns,
without assuming a canonical checksum. The author independently verified the
constants, inequalities and all 48 sealed P evidence file hashes.

## Source coverage, registration and preservation

P discovery is readiness only and receives no Source implementation credit.
Existing complete stock adler32.c/public _adler32 already implements the named
algorithm in a separate bsp_zlib121 library. Its actual cdecl section is 284 B,
not this 297 B ECX/EDX/stack/RET4 binding with the same frame/alias/flag effects.
No stock API/callback/caller adoption or Original compiler-option/version claim
is made. The sealed full component proof and current worker/Root/Main/origin
reference checks were read before code; all five prospective paths were absent
before output creation, with exact revisions recorded. Fresh whole typed/live/
installed-PE/instruction capture verified C:/Users/sqz269/bsp.gpr,
/battlestationspacific.exe, base 00400000, before Source.

Strict /std:c++17 /MD /O2 /W4 /WX /fp:strict /Gy and actual Release|Win32 C++17
objects each have one complete executable section, symbol offset zero, no
helpers, undefined providers or code/data relocations. Every data byte,
symbol/auxiliary and relocation is recorded. Both Core linker dictionaries
uniquely resolve the decorated symbol to the unique real matching object.
Required scripts/build.ps1 session 68812 exited zero in 18.3602053 s; existing
CTest checks passed 2/2 in 7.42 s. No compiler/Source rejection, new tests, probes,
Native execution or dynamic fault tests occurred.

Independent N freeze was confirmed before any tracked/cache/Git change. Before
Source/build, 1519 prior N/K/J/G/E/B5 input files, 1,823,888,909B, were copied with
hashes, including full packet trees/current objects/Core/project/tlogs. N local
artifacts remain immutable and all six prior real providers remain unchanged.
Full P copy is 50 files / 357343 B; its verified sealed set is 48 files / 333771 B,
plus two administrative files. All three supplied P pins match.

## Clock and primary review

The full clock begins at P's 2026-10-16 08:48:20 UTC: readiness 943.721305 s through
09:04:03.721305, administrative gap 121.278695 s, Q implementation from 09:06:05.
All copying/hashing, qualification, Source/compile/build, static proof, full
artifact/dictionary review, docs/ledger/index, commit/release/handoff overhead is
included; warm build time is separate within that total. No speedup claim is
made. The final ignored receipt pins the complete author endpoint and commit.
No Ghidra write occurred. Provisional physical prototype/comments are supplied
for Root after independent review, preserving adler32 and previous annotations.
Root owns acceptance/integration; Original full ABI/game validation is unproved.
