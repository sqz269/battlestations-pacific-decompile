# Native PE image queries (EK)

This packet reconstructs the complete original `__ValidateImageBase`
(`00C16D50..00C16D78`, 41 bytes / 14 instructions) and `__FindPESection`
(`00C16D80..00C16DC1`, 66 bytes / 29 instructions). The correct Visual Studio
2005 library names remain in Ghidra. The source is
`src/native_crt_pe_image_queries.cpp`; its declarations are in
`include/bsp/native_crt_pe_image_queries.hpp`. Both are MSVC Win32 naked cdecl
entries with only the original pointer and, for section lookup, RVA arguments.

The complete native schedules, fresh PE/live equality, nine original caller
sites, compiler inputs and emitted/archive/linked comparisons are recorded in
`reports/native_crt_pe_image_queries_ek.json` and the immutable local packet.
No helper or original caller is executed. The static link driver is never run.
The existing math tests do not exercise these entries.

## Original entry and memory contract

Let entry ESP be S. Validate reads its pointer from `[S+4]`. It does not save
registers or create a frame. Its plain RET leaves ESP=S+4; the caller removes
the original argument word. EAX is a 32-bit BOOL, not a C++ bool. The first
read is the DOS signature WORD. On MZ, it reads DWORD `[base+3Ch]`, adds the
base with 32-bit wrap and reads the NT signature DWORD. Only on PE does it
read optional-header magic WORD `[NT+18h]`. No machine, header-size, image-size
or bounds validation is added.

An early rejection uses XOR EAX,EAX and leaves ECX equal to the supplied
image pointer. The optional-magic path clears ECX, compares against 10Bh,
sets CL and copies ECX to EAX. Its flags remain from that final 16-bit CMP,
including when the result is false. EDX, EBX, ESI, EDI and EBP are untouched.
All paths preserve DF. A fault exposes the registers at the faulting read;
there is no local exception handler or translation.

Find reads base from `[S+4]`, derives NT using the original wrapping addition,
then reads optional-header size WORD `[NT+14h]` before reading section count
WORD `[NT+6]`. The count read follows PUSH EBX and PUSH ESI. It zeroes EDX,
tests the count, pushes EDI and forms `NT+18h+optional_size` with LEA. The
original JBE after TEST takes the zero-count path without loading the RVA.

For a nonzero count, `[ESP+14h]` after the three pushes is the original RVA
at `[S+8]`. Each 40-byte section is visited in order. The leaf first reads
VirtualAddress at +Ch; it reads VirtualSize at +8 only if the unsigned RVA
is at least that address. It returns the first section for which
`RVA < wrapping_u32(VirtualAddress + VirtualSize)`. A wrapped endpoint is
not repaired; raw-data size is not substituted. Section pointer increments
also wrap. Header signatures, access permissions and pointer validity are
the caller's responsibility.

On a match EAX is the actual section-header pointer, EDX is its zero-based
index, ECX is its VirtualAddress, and flags are from CMP RVA,end (CF=1).
A miss returns EAX=0 with XOR flags (CF/OF/SF=0, ZF/PF=1, AF undefined).
On a zero-count miss EDX=0 and ECX=NT; on an exhausted scan EDX=count and
ECX=the final section's VirtualAddress. POP EDI/ESI/EBX and RET preserve
these flag effects, restore the nonvolatiles, leave EBP untouched and return
with ESP=S+4. The caller removes its two words. DF is unchanged. The source
retains read order, partial stack saves on faults and ordinary return paths.

## Caller and ownership boundaries

Three original calls to Validate and six calls to Find are checked against
fresh live bytes and the original PE, including their ADD ESP,4/8 cleanup.
`__ValidateEH3RN` and `_ValidateScopeTableHandlers` supply image pointers
through their actual caller state. `__IsNonwritableInCurrentImage` separately
passes fixed 00400000 and subtracts it from the target address. EK introduces
no fixed base, global, image allocation, callback, binding or header copy.

The separate sealed readiness packet remains the authority for the blocked
wrapper: its full 187-byte body includes a 23-byte inherited-frame filter and
landing omitted from the saved function listing. Its E03738 EH4 scope table,
fixed original header domain and C07C90 FS-dispatcher cycle remain unowned.
The current game uses image base 10000000, and the published canonical RO/RW
mappings do not establish original PE headers at 00400000. D6A618's original
C06AC8 code-pointer value does not establish callable code at that target.
Neither EK leaf resolves these ownership or exception-dispatch dependencies.

## Validation and limitations

The required strict Win32 build, eight original seed comparisons and two
existing CTests are retained with their complete logs. Static proof binds
the exact current source and consumed header to one CL command/read/write/Fo
record, the current object to one archive member, and both entire original
bodies to the never-executed forced image. There are no native-body
relocations, imports, globals, external providers or extra writable symbols.
The ordinary game has no EK consumer and omits the two leaves.

The command and read records each name only this source. The one matching
write record is MSBuild's aggregate group for 1,283 sources and outputs; the
owned source and its expected /Fo object each occur exactly once. The full
aggregate record is retained, with no second matching or stale write group.

All attempted commands and rejected states are retained in the local packet.
The handoff pins the clean source commit. Exact recursive local payload and
the Windows-path-unique consumed external set are hashed with SHA256 and
SHA512 twice; the two inventories are pinned separately to avoid self-hash
recursion. This is complete original-entry source and static ABI/byte proof,
with existing build tests, not runtime validation, caller adoption, exception
closure, original-address code installation or gameplay validation.
