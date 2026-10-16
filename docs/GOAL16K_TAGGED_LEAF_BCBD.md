# GOAL16K: six complete integer entries

Six readable C++17 definitions reconstruct the complete entries below. All
strict and actual registered MSVC32 executable sections equal their entire
live/installed-PE Native bodies: six functions, 22 bytes, 12 instructions.
No Native-size prefix, hidden helper, guessed class or narrowed input domain is
used. Names/formal C++ types are new observed-operation hypotheses, not recovered
Original symbols or type spellings. Full evidence is in
`reports/goal16k_tagged_leaf_bcbd.json`.

| Address | Native bytes | Observed complete operation |
|---|---|---|
| 00BC04B0 | `33c0c3` | XOR EAX,EAX; RET |
| 00BC3030 | `8b4114c3` | MOV EAX,[ECX+14h]; RET |
| 00BC3080 | `8b4168c3` | MOV EAX,[ECX+68h]; RET |
| 00BC3100 | `8b4120c3` | MOV EAX,[ECX+20h]; RET |
| 00BC7A60 | `8b4108c3` | MOV EAX,[ECX+8]; RET |
| 00BD1830 | `b001c3` | MOV AL,1; RET |

## Physical inputs, flags, storage and limits

The four getters consume only ECX as raw backing, read exactly one current
DWORD and return those bits in EAX. ECX and all other registers/flags remain
unchanged. Address addition wraps modulo 2^32; unaligned backing is allowed by
the measured x86 instructions. There is no backing-null guard, alignment/range
check, invented whole-object extent, typed field ownership, canonical global,
provider or data store. The faulting load preserves preceding registers/flags;
no prologue, spill, argument-slot read or extra memory access is inserted.

BC04B0 reads no input register. XOR clears all EAX, sets CF/OF/SF zero and ZF/PF
one, with AF undefined; other general registers are unchanged. BD1830 reads no
explicit input but retains the incoming upper 24 EAX bits while setting AL=1.
It preserves every flag and other general register. The new byte-return
interface emits exactly that AL-only schedule, not a full EAX=1 normalization.

All six use plain RET, reading the current return word and advancing ESP by 4.
They read/clean no stack argument words; formal Original unused-argument count
or C++ type spelling is not recovered solely from a body. Source interfaces
bind only the observed inputs. There are no prologues or hidden stack accesses;
backing/return-slot aliases and literal fault prefixes remain intact. No x87 or
SIMD instruction occurs, so FP state and x87 FIP are unaffected.

Unsigned uintptr_t arithmetic and memcpy word transfers express the getters
without pointer-extent, alignment or signed-overflow assumptions. Source-level
defined behavior still requires accessible bytes; complete measured code proves
the current artifact's invalid/unaligned/wrapped address, register, flag, alias
and fault-prefix effects. It does not prove identical mapped fault/return EIP
placement, Native SEH/runtime compatibility, caller adoption or installed/game
behavior. Raw formal pointers and byte/full-word return spellings are hypotheses.

## Bounded discovery and existing Source

Readiness screened 39 indexed tagged/no-whole-function-record/zero-indexed-callee
names in BC-BD and inspected 18 complete bodies/309B, below the 20-body cap.
Twenty-one zlib names were not opened; correct library names remain unchanged.
Fresh typed bodies have complete ranges and no flow overrides. Each live CLI
body matches the installed PE after verifying C:/Users/sqz269/bsp.gpr,
/battlestationspacific.exe, image base00400000, in each capture batch.

BC3030 has abstract/caller-comment coverage; BC3080 and BC3100 have caller-local
field reads, and BC7A60 a caller declaration comment. These are mapped explicitly
and are not standalone physical entries or evidence of concrete parent wiring.
All four new paths were verified absent in worker, current Root/Main and origin
before creation; exact revisions are recorded. No other Source was changed.

Four POD8 kernels/186B already exactly match existing whole callable Source
providers 00419960/0041C7E0/0041C680/0041DAC0, including complete current actual
sections and unique real archive members. They are not duplicated or counted
new; explicit alias/binding scope would be needed for additional address records.
Three FLD getters/12B belong to Root's disjoint MAX/16L packet. Three raw D68380
stores/21B remain a separate ABI followup. BC3EC0/BCFB20 total68B and contain
actual import/virtual lifecycle calls absent from the call index; their dynamic
targets remain unqualified. The report retains these classifications without
promoting zero indexed callees to leaf proof or inventing vtable/global owners.

## Complete objects, archive and preservation

Strict /std:c++17 /MD /O2 /W4 /WX /fp:strict /Gy and registered Release|Win32
/std:c++17 objects each contain exactly six executable sections/22B, all symbols
at offset zero, no helpers, executable relocations or undefined providers. All
sections, data bytes, symbols/auxiliaries and relocations are recorded. Each
object has six type7 debug$F metadata relocations to its own function symbols.
The full Core archive contains one real matching K object. Both its big-endian
first and little-endian second linker dictionaries uniquely resolve every K
decorated symbol to that same real member; all dictionary offsets/indexes were
validated, with dictionary counts/hashes and selected resolutions retained.

Required scripts/build.ps1 session 94928 terminated with exit 0 in 17.8731023 s.
Existing reconstructed_math and tool_tests checks passed 2/2 in 7.44 s. The initial
strict compile succeeded and matched all 22 B; no Source form or compiler failure
was rejected. No new tests, probes or Native execution were added/performed.

Before any new Source/compile/build, 322 complete files/452,017,396B were copied
with hashes under local/goal16k_tagged_leaf_bcbd/frozen_prior: full prior B5/E/G/J
trees, current real Core archive/objects/project/tlogs, Source/header/docs/reports,
pre-K CMake and all K readiness files plus receipt. A preservation wrapper first
stopped on older report path-key differences, then used explicit module mapping;
that setup correction preceded copying/compilation and is retained in the clock.
Current B5/E/G/J Source/header/report and unique real provider pins remain intact.

## Time, annotations and remaining review

First readiness clock 07:33:51 UTC; sealed readiness 737.300032 s, scheduling gap
76.699968 s, implementation start 07:47:25 UTC on 2026-10-16. Full time continues
through preservation/correction, strict/warm build, both dictionaries/providers,
docs, ledger/index, commit, release and final author receipt. Warm build time is
included separately; no throughput or controlled speedup claim is made.

Six owned name/reconstruction records carry operation, Original physical ABI,
evidence and uncertainty. Existing correct library names in other scopes are
retained. No Ghidra name, prototype, comment, tag or export was changed. The
report proposes provisional physical prototypes/comments for Root after
independent review, preserving previous values. Final local receipt pins the
commit and complete inclusive author interval. Original full ABI, parent
adoption and game validation remain unproved; Root owns review/integration.
