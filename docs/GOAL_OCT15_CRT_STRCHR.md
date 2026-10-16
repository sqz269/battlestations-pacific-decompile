# Original CRT character search

Address: `00BF86F0`; retained correct library name `_strchr`.

The complete Original algorithm has two body ranges:
`[00BF86E0,00BF86E5)` is the5B/3op unaligned-match return tail,and
`[00BF86F0,00BF87AE)` is the190B/86op entry body. Total195B/89 instructions,
no calls or global/locale/TLS dependency. The206B physical envelope also
contains11 unreachable gap bytes,which receive no executed-instruction credit.
Typed metadata confirms the pre-entry tail is defined inside this function;
the CLI's function-head listing omits that earlier range,so its bytes/three
instructions were checked separately. No listing/flow repair was performed.
All206 physical bytes equal live Ghidra and the installed PE.

| Routine | Coverage | Native interface |
|---|---|---|
| 00BF86F0 | Complete Original195B/89op byte/word algorithm | cdecl(pointer,int),plain RET,raw EAX match address/0;EBX/ESI/EDI preserved |

The Source interface binds actual parameter words. Native pointer is at P+4
and value at P+8,but capture order is the low value BYTE FIRST,then pointer
DWORD. The remaining three value bytes are not read. The pointer is retained
through the whole scan;there is no implicit pointer/name owner,callback,SDK
search,ASCII-only restriction,extra null guard or new failure recovery.
Binding addresses stay stable and Source metadata is disjoint from reached
cells/payload. Reached payload aliases remain admitted. The new ordinary C++
interface is not the native cdecl argument-home/binary replacement.

## Complete byte/word schedule

While the retained cursor is unaligned,read one current BYTE,increment cursor
with DWORD wrap,compare to retained sought byte,and only then test NUL. A match
uses the pre-entry tail to return cursor-1. This path does not read a DWORD;
the pseudocode's widened unaligned read is a partial-register artifact.

At alignment,replicate the sought byte into a DWORD. Each iteration reads one
current aligned DWORD. With W=captured word,V=W XOR repeated-byte and
M=7EFEFEFFh,the candidate expression is `(~V XOR (V+M)) AND 81010100h`.
The original-word sum W+M is separately retained. Increment cursor by4 before
the candidate branch. Every addition/address/subtraction uses DWORD wrap.

A nonzero candidate causes a genuine SECOND DWORD read at cursor-4. Test the
four bytes of that retained second word in increasing address order,matching
before testing zero for EACH byte. Return the match address or0 at the first
reached terminator. If none qualifies,continue at the next DWORD. Do not replace
this with four fresh byte reads or reuse the first DWORD snapshot.

When no match candidate exists,compute `(~W XOR (W+M)) AND 81010100h`.
Zero repeats. A nonzero01010100h subset returns0. Otherwise the original sum's
high bit controls compensation:zero returns0,nonzero repeats. All bits,including
80h/FFh and an integer search value with ignored upper bits,remain admitted.
Searching zero returns the terminator address because match precedes NUL.

Aligned reads may include up to three bytes after a terminator;the entire
reached DWORD must be readable. The admitted ordinary storage domain preserves
these widths and rereads. Invalid-memory timing,flags/register/private stack,
native runtime faults and ownership/lifetime are separate proof obligations.

## Native closure and evidence

There are no outgoing native calls or computed transfers. All conditional/
unconditional branches target listed instructions within the two body ranges.
There are no absolute-memory operands requiring globals or relocations. The
representative real incoming row is `0092523A CALL 00BF86F0`,in `009251F0`;
its full native caller was reviewed,pushing5Ch then retained name and consuming
eight bytes afterward. No complete-body claim is made for the other callers.
The general qualified-node parent and Original strnicmp/locale/TLS dependencies
remain NOT ready;this packet supplies no parent implementation credit.

The correct library identity and prior comments are retained through owned
evidence,locked annotation,project save and export/readback. Exact reports
separate this genuine Original algorithm from a current-SDK projection.

## Approved focused differential probe

After primary full-assembly/Source review and explicit approval,one local
probe compiled and passed all72 raw-pointer pairs. It copied the genuine206B
physical envelope at BF86E0,whose SHA256 is
`80c4de8b7a904f4fc5015fae8e37f82934196cf010f0872fc4d36071a239d111`.
All89 listed instructions were checked for internal relative branches,no
calls and no absolute-memory/global operands. Allocate RW,copy exact bytes,
protect RX,flush instruction cache,and invoke code+10h as
`char* (__cdecl*)(char*,int)`. Compare raw result bits with the compiled Source
using actual two-word parameter backing on the same buffer.

The single probe has three padded valid templates:empty NUL,mixed ASCII and
high-bit/mask bytes. Four starting alignments and six integer needles
(0,5Ch,80h,FFh,100h,-1) yield72 comparison pairs. Use the safe filename
`bsp_strchr_original_probe.exe` with `/link /MANIFEST:EMBED`;retain its Source,
exact copied code and log under ignored local storage. No CTest/framework or
broad suite is added. The final compile keeps linker flags on one response-
file line after correcting an initial local command-scoping issue. The actual
PE embedded asInvoker manifest was verified. This checks only reached valid
byte/word behavior,not invalid storage,private frame,ABI,register/flags or game.

Primary independent full195B/89op assembly and entire Source/header review
passed. The reviewed Source/header SHA256 pins were asserted unchanged before
recording its receipt and before/after the focused probe. Exact Original bytes,
probe Source/executable/objects,compiler command/version output,embedded
manifest,build/run logs,all72 results and SHA256 pins are retained under local/.
Final receipt: `local/goal_oct15_crt_strchr_probe.receipt.json`, SHA256
`65511439cd4b765cde31c542b6cf242e9c7eb77ae4233080be56ff52ec92ff62`.

Locked annotation preserved the correct library name and all prior comments,
recorded prior values in `local/ghidra-annotations-20261016T015108Z.json`
and `local/ghidra-annotations-20261016T015720Z.json`,saved the project and
refreshed exports/readback. The final readback retains the original library
comment and both evidence stages. Whole Original algorithm Source
is ready through its new interface;the actual primary combined registered
normal Win32 build/emitted-object gate remains pending. No worker CMake queue
or duplicate normal build is used.
