# Native MPAK enumeration and member-directory selection

Addresses: `00BB5F40`, `00BB68F0`, `00BB40C0`, `00BB79E0`, `00BB79F0`,
`00BB7A00`, `00BEE340`.

Names below are hypotheses, not recovered symbols. The implementation uses the
actual 44h provider and the records established by `BB8240`, `BB7C50` and
`BB6870` in `docs/NATIVE_MPAK_PROVIDER.md`. It adds no parallel archive owner,
string collection or allocator domain. The container search remains an explicit
library contract. These are new source interfaces, not original ABI/FH3 entrypoints.

| Routine | Inclusive native body | Original ABI | Coverage |
| --- | --- | --- | --- |
| Append names BB5F40 | BB5F40..BB60C5, 390 bytes | ECX provider; prefix, extension, flags, output stacked; RET10h | complete |
| Select member directory BB68F0 | BB68F0..BB69F8, 265 bytes | ECX provider; input, output stacked; AL Boolean; RET8 | complete with original library search contract |
| Probe default BB40C0 | BB40C0..BB40C4, 5 bytes | ECX unused; ignored stack word; AL=0; RET4 | complete |
| Operation default BB79E0 | BB79E0..BB79E4, 5 bytes | ECX unused; four ignored stack words; AL=0; RET10h | complete |
| No-op default BB79F0 | BB79F0, 1 byte | RET; no stable return value | complete |
| Clear result BB7A00 | BB7A00..BB7A16, 23 bytes | ECX unused; output, ignored word stacked; EAX output; RET8 | complete |
| Native filter BEE340 | BEE340..BEE38F, 80 bytes | ECX prefix, EDX extension; flags, candidate stacked; AL Boolean; RET8 | complete using existing actual string helpers |

The five previously undefined functions were reported to the primary integrator
with these exact ends. The primary defined, read back and force-exported them
under its write lock (`reports/native_az_function_definitions.json`). This worker
performed no Ghidra mutation. BB5F40's verified listing has 136 instructions and
no gaps. Every direct call site and its actual containing body are recorded in
`reports/native_mpak_enumeration.json` and checked mechanically.

## Enumeration and filter

BB5F40 first makes an actual pooled copy of the prefix. If its recorded length
is nonzero and its last byte is not `/`, it constructs a slash string and grows
the temporary to append it. The slash's length/data are captured before growing
the prefix. Normal slash return uses those captures, while unwind destroys the
current slash header. Both temporary and original prefix remain distinct.

At BB6060..BB6070, EDX comes from entry stack argument 2; after pushing candidate
and flags, ECX comes from entry argument 1. Thus the filter receives the original
prefix, not the slash-appended temporary. A prefix `foo` with recursive flag1
can match `foobar/b.lua`. The full stored name is appended, with existing output
contents preserved. No basename extraction or sorting is added.

File begin/end are provider+20h/+24h. Count is the unsigned interpretation of
signed wrapped byte difference divided by 24h. The loop repeats current bounds
for the original invalid-parameter diagnostic and reloads begin after a returning
handler. Index and byte offset advance separately. It does not dereference the
file's numeric or offset-vector fields for enumeration.

BEE340 first calls existing actual-header `43E9A0(candidate,prefix,0)`. If flags'
low byte is zero, existing actual `4BCB80(candidate,"/",INT_MAX)` must return an
unsigned index no greater than the CURRENT prefix length; absent slash returns
FFFFFFFF and normally rejects. Finally it subtracts CURRENT extension length
from CURRENT candidate length with DWORD wrap and calls43E9A0 at that offset.
Pattern NUL termination, a null candidate-data rejection, and the borrowed
writable null-pattern bytes E17BF0 are inherited from43E9A0. Unlike the existing
bounded string_view projection, this actual-header body does not reject empty
prefixes or embedded NUL as an unsupported domain. Static slash literal CE7898
was verified to contain `2F 00`.

Native FuncInfo DFDE10 references two unwind actions: state1 CC44B8 ->41DD20 on
slash, then state0 CC44B0 ->41DD20 on the prefix. The initial prefix copy occurs
before state0 is armed. Normal explicit string returns capture data and size
before the pool getter. The source preserves the state transitions and reuses
41DD20 for unwind. Original FH3/SEH and simultaneous cleanup exceptions remain
outside the source interface's proof.

## Member-directory selection

BB68F0 copies its input name into the first eight bytes of a temporary 14h
directory record, then zeros that record's actual0Ch member vector. It captures
the directory end at provider+34h before its first iterator diagnostic; begin
at provider+30h is read afterward and has a separate diagnostic.

BB4F40 receives captured first in ECX, captured end in EDX, and the temporary
record on the stack (RET4). Its body BB4F40..BB4F73 walks14h directory rows,
passing row+8 and the key to5EFBA0. It stops for a signed result greater than -1.
5EFBA0's body 5EFBA0..5EFC15 searches the row's eight-byte string elements for
equal recorded lengths and then empty/empty or CRT `_stricmp` equality. It
captures vector begin/end and reloads begin when deriving the found index.
Neither routine compares the key with the directory's own name. BB5330's other
call at BB533C forwards the same first/end/key contract into BB4F40.

`NativeMpakDirectorySearchLibrary` preserves that exact search boundary; no
implementation of the original container specialization is supplied or counted
as reconstructed here. The native BB4F40 and5EFBA0 instructions can be used as
the library binding in the isolated comparison fixture, while their broader
source ownership/classification remains external to this packet.

After search, the selector always destroys the temporary with existingBB6500
before checking the CURRENT directory end and iterator diagnostics. A miss
preserves both selected pointer provider+3Ch and the output. A hit publishes
the found record at +3Ch before copying that record's own name into the live
output header. Output identity with the found record skips copying. There is
no explicit lower-bound check on the found iterator and no added check. Native
BB68F0 has no EH prologue: if the search throws, the temporary is not cleaned
up by this routine. The source adds no cleanup frame for it.

## Genuine defaults and prior correction

The live D641F8 table has +08=BB5BB0 (actual open; sibling packet),
+0C=BB79E0, +14=BB5F40, +18=BB40C0, +1C=BB5540, +20=BB7A00,
+24=BB68F0, +28=BB79F0. Existing BB5540 is already implemented in
`src/native_vfs_lookup_leaves.cpp`; it is neither duplicated nor counted here.

The AY follow-up text in `docs/NATIVE_MPAK_PROVIDER.md` suggested BB79E0 and
BB7A00 as lookup/open methods. The exact five-byte false/RET10h body at BB79E0
and the 23-byte output clear/RET8 body at BB7A00 correct that interpretation.
BB7A00 writes five DWORDs in order +10h,+Ch,+8,+4,+0: exactly14h bytes, or
20 decimal bytes, not20h. It preserves all bytes beyond +13h. BB40C0 is a
genuine false/RET4 default. BB79F0 is a genuine bare RET.

## Verification and boundaries

Strict `scripts/build.ps1` Win32 Release passed with warnings as errors and
`MSBUILDDISABLENODEREUSE=1`; both existing CTests passed. All29 reported calls
passed the live verifier. The focused native/source comparison passed with
48 string allocations and48 releases. Flags0/256 selected only `foo/a.lua`;
flag1 also selected `foobar/b.lua` and `foo/deep/c.lua`.

Current project/program verification uses the BSP CLI against
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`. The report carries exact
body hashes, direct-call rows, strict Win32 build/CTest results and fixture
artifacts. The focused ad hoc comparison uses 11 disk-derived native bodies,
all byte-matched against live Ghidra, relocated only for calls and referenced
constant strings. Prefix/file/member input records are preserved before
execution and results afterward under `local/`.

The comparison covers the original-prefix behavior with recursive/nonrecursive
flags, flag-low-byte handling, directory membership with case-insensitive key,
miss preservation, output self-alias, selected-pointer publication before output
allocation, empty-prefix/no-slash filtering and the four genuine leaves. Native
enumeration/selection use existing source string/vector/destruction dependencies
as explicit fixture bindings; this does not independently prove those bodies.
Native EH is not exercised by the copied-instruction fixture. Arbitrary aliases
into original stack spills, concurrent mutation, malformed-pointer faults and
installed archive/gameplay reachability are unproven. The original installation
and saved analysis were not modified by this worker.
