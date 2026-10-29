# Independent review of the recursive Dyn profile report

Candidate `cc32f14469be8ef3b6280af181bd9a01c7d7ccee` is suitable for normal
C++ source integration, with the existing unwired/new-ABI qualifications and
the nonblocking symbol-receipt correction below. No blocking source mismatch
was found. This review adds no reconstructed body, vendor, activation or
runtime credit. It owns only this document and its JSON report.

The review tree was created from frozen `origin/main`
`1e10d9de40bda6596ebdb89487a657cdba034d8d`. The candidate's four committed files,
all 220 original bytes/74 instructions, its two retained objects and all
thirteen listed validation artifacts were inspected or hash-verified. The two
retained source snapshots equal the exact committed source/header bytes.
The author's source tree was clean. No source edits or address lease were taken.

Every live analysis query used the verifying BSP client for project `bsp`,
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, x86 LE32,
image base `00400000`. Fresh live bytes match the installed PE: C320D0..C321AB
is 220B, SHA256 `442ca9b5b112ec95c7324c9765ba069196762087ad5a4e5a04843c1ab612bc28`.
The executable SHA256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

## Findings and integration boundary

**N1 — informational receipt correction.** Candidate
`validation.only_external_function_dependencies` lists a standard compiler
security cookie. The actual strict, Release and fresh review objects have only
two undefined symbols: real `__alldiv` and real
`cc15_dyn_live::format_string_1024_0040cfc0`. There is no cookie symbol,
relocation or check in this function. Remove that unused entry when refreshing
the metadata; no source repair is required. No stack-protection guarantee is
inferred here. The candidate already qualifies frame/fault/exception identity.

Accepting this normal body does not establish an incoming native call, supply
a sink, bind the real publication/scale cells, preserve the old binary ABI or
prove runtime behavior. Those remain explicit integration obligations. There
is no source-level partial-construction cleanup dependency here: this body
reads actual existing profile nodes; it does not call C50390 or construct one.
The separately pending C50390 repair is not assumed as accepted evidence.

## Storage, denominator and captured values

The candidate uses the existing 48h `DynProfileNodeStorage`. The existing
1Ch `NativeLegacySboStringStorage` starts at node+10h with its leading word
preserved: buffer at node+14h and capacity at node+28h. These offsets are
independently consistent with the actual type and its static assertions.
No replacement class, pooled-string accessor or constructed profile is used.

Native first reads `[*0109E9F8+10h]`. The real profile layout defines that as
cache slot 1 (`Ch+4*id`), not a substituted root/current node. The candidate
borrows the genuine stable publication-cell address and captures that
denominator-node pointer before comparing the name capacity. Its denominator
tick qword is read later by the actual FILD at the native epoch. Owner values
may change through callbacks and are reread on every recursive invocation;
the caller's cell address and context bindings must remain valid/stable.

Capacity is compared unsigned against 16 before the low/high tick reads.
The two raw node+38h/+3Ch words are captured in order, used for both signed
FILD and signed integer division, and never converted through C++ arithmetic.
CF survives the MOV/FILD/FMUL/FILD/FDIVP sequence until the name branch:
capacity<16 chooses the inline address; otherwise the heap pointer is read.
There is no second capacity read or accessor that could move that epoch.

## Floating-point and integer helper proof

The complete emitted function contains precisely the five intended x87 ops:

```text
FILD captured signed qword
FMUL qword at the actual D7A220 double cell
FILD qword at captured denominator-node+38h
FDIVP ST1,ST0                     (DE F9)
FSTP qword directly to [ESP]      (DD 1C 24)
```

The percentage remains unspilled until the printf argument slot, after the
SBO pointer decision. There is no extra load/store, host double arithmetic,
PC24/PC53 choice, zero guard, clamp or FP-state reset. Live D7A220 bytes
`0000000000005940` establish the required double 100 binding. Ambient
precision/rounding/masks/status and zero-denominator effects are retained by
the original operations. CFG analysis finds maximum added x87 depth 2 and
zero added depth at all four calls and the exit. Unmasked fault transport and
fault/frame identity remain outside the new C++ contract.

The raw double argument stays on the stack through integer division. The
divisor is pushed high=0/low=1000, then captured dividend high/low. The actual
helper is signed64 division, callee RET16, quotient EDX:EAX; only low EAX is
pushed as Time's raw `%d` word. The +40h call count is read afterward. The
private ASM-only declaration selects symbol `__alldiv`; it is not a normal
cdecl call interface with four C++ arguments.

The review independently parsed the current `/MD` MSVC 14.51.36231
`msvcrt.lib`, SHA256
`863545158568f88be2b64aa98f00ea69c5f9a7dc1e7e7a281af9debaf24e7918`.
Its actual `lldiv.obj` member defines `__alldiv` at offset zero in a 170-byte
section with zero relocations. All 170 bytes match fresh live BF7D40..BF7DE9,
ending `C2 10 00`; code SHA256 is
`8964dcf0d769cb61dd4354feac00491ae8c1b34eb9627a4c5ec2d4350243d395`.
The independently extracted object is also byte-identical to the author's
retained runtime object. This is a genuine vendor dependency, not a stub or
new in-house body. No runtime executable was linked or run.

## Formatter, sink and traversal epochs

The compiled 35-byte format constant exactly matches native D79DA4, including
the final four percent characters and null. The only formatting call is the
existing real cc15 formatter, which calls `vsprintf_s(buffer,400h,format,va)`.
Its existing native-vendor/context/locale qualification is inherited. Native
return/error handling is not augmented. The 400h text buffer remains borrowed
storage for the invocation and stays alive while descendants run.

Argument accounting is exact: the double is 8B; destination, format, name,
calls and low quotient add 20B. The formatter returns with all 1Ch bytes still
present. Shared second is incremented with the actual memory ADD, second is
read before first, then ADD ESP,1Ch precedes the single text push. The indirect
sink receives ECX=first, EDX=updated second and a text stack argument. Genuine
fastcall RET4 cleanup restores the manual stack. Aliased first/second word
semantics therefore match at that boundary.

Child data and count are first loaded after the sink, data before count.
Each child gets a fresh local formed from the **current** parent first+20,
the same second reference and same sink/context/scale bindings. The child
pointer is read after the local is formed. Post-child count is reread before
data; the captured cursor advances four and the end uses wrapping data+count*4.
Neither cursor reset nor count/less/cycle guard is introduced.

The emitted initial loop condition optimizes cursor==cursor+count*4 to
`TEST EAX,EAX` on wrapped count*4; uint32 wrap makes this equivalent even when
count*4 wraps to zero. Post-child code advances the cursor between the count and data loads;
the volatile field-read order remains count then data. Register/fault-epoch
identity is not claimed. The live end comparison and old cursor are retained.
The recursive new cdecl call passes six words and removes 18h itself, rather
than pretending to preserve the original native RET0Ch interface.

## Independent object evidence and limits

An exact committed source/header copy was compiled in ignored review storage
against the current review tree's real headers, using MSVC x86 19.51.36244.0:
`/std:c++17 /permissive- /EHsc /fp:strict /O2 /Ob2 /MD /W4 /WX`.
It passes without warnings. All 242 function bytes (92 instructions) equal
both retained strict and standard-Release candidate functions, including the
EBP-based local addressing. Function SHA256 is
`e6006bd2976a2ae86f702ccfd7e87bb9e8bd06ec961ad615fbc0da675f196413`.
This is a current-dependency compiler consistency check, not original-body
byte-identity or runtime-equivalence proof.

Full manual stack/x87 CFG joins pass; helper RET16, sink RET4, formatter 1Ch
and recursion 18h cleanup were checked in emitted code. The three direct
original call rows pass the live actual-call verifier; the indirect boundary
was independently read at C32157 and checked in the object. Formatter,
storage and context definitions used here have no delta between the candidate
and the frozen current origin. Four real function/data relocations resolve
to the genuine helper, real formatter, recursive public function and exact
format constant; no provider is a returning placeholder.

The JSON report pins source, native, runtime, object, dependency and raw receipt
hashes. No game/probe/native run, new test, shared CMake/ledger/source edit,
Ghidra mutation, activation or original-ABI adapter was performed. Incoming
caller/sink/cell custody, binary ABI, exception/fault/stack-exhaustion identity,
concurrent mutation and gameplay remain unproved integration/runtime work.
