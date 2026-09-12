# Actual PARTICLE definition base

Addresses: `00B01150`, `00B00C20`, `00B00FB0`, `00B01130`, `00B00920`, `00B00740`.

The new C++ interface operates on the actual sparse 80h PARTICLE definition
prefix, its eight-byte name at +08, and its 0Ch array descriptor at +68.
This is distinct from the emitter definition constructed by `00AFA280` in
`native_particle_definition.hpp`. It borrows that module's SAME
`NativeParticleDefinitionBindings` for the string storage, actual F8D344
parameter pool, reference profile and matching allocation/free domains.
No additional owner, vector, pool, parser or application vtable is created.

| Routine | Proposed descriptive name | Original ABI | Coverage |
| --- | --- | --- | --- |
| B01150..B012E8 | BSP_ParticleTypeBase_Construct | ECX owner; stack(name8h, word14, parent18); RET0C, EAX owner | complete native body; explicit input for otherwise unspecified stack word |
| B00C20..B00CA4 | BSP_ParticleTypeRecords_Reserve | ECX actual descriptor; stack signed capacity; RET4; EAX unspecified | complete |
| B00FB0..B0112F | BSP_ParticleTypeBase_Destroy | ECX owner; RET; EAX unspecified | complete native body including omitted post-free tail |
| B01130..B0114D | BSP_ParticleTypeBase_ScalarDeletingDestructor | ECX owner; stack flags; RET4, EAX captured owner | complete |
| B00920..B00940 | BSP_ParticleTypeBase_ClampRecordRange | ECX owner; RET, EAX new +54 | complete raw leaf; no Ghidra function at worker capture |
| B00740..B00742 | BSP_ParticleTypeBase_NoOpWord | ECX ignored; stack one ignored word; RET4, EAX untouched | complete raw leaf; no Ghidra function at worker capture |

Names are descriptive hypotheses, not recovered symbols. These are new C++
interfaces, not replacement original ABI exports. The code preserves recovered
member cleanup order for C++ exceptions, but does not implement the original
FH3 personality, exception metadata, SEH delivery or invalid-pointer faults.
The transplanted native differential probe executes normal paths only.

## Producer and layout evidence

The constructor's full listing establishes EBX=0, EBP=actual owner and
ESI=owner+68 throughout the initializer and descriptor append, except that
ESI becomes the stack-record source only in the final REP MOVSD path. All five
direct native constructor callers (`AF89F4`, `B058F4`, `B07704`, `B08844`,
`B0A0C4`) forward the same three words and their actual owner. Their enclosing
functions end with RET0C. The constructor writes +00 first to CEB130, sets +04
to 1, and then installs D5DDC0. It zeroes the name and descriptor before its
self-name test. The common base never initializes +10.

| Actual owner bytes | Recovered initializer |
| --- | --- |
| +14/+18 | caller word and actual parent pointer, without retain |
| +20/+24 | current raw CE3804 / D7A24C DWORDs loaded using MOVSS |
| +1C,+2C,+30,+48,+34,+38,+3C,+40,+44,+74,+50,+54,+5C | zero DWORDs, in the listed native order (byte +4C lies between +74 and +50) |
| +58 | positive zero DWORD |
| +29,+28,+4C,+60,+61,+63,+65 | zero bytes |
| +62,+78 | one byte |
| +68,+6C,+70 | array pointer, signed count, signed capacity |

All other bytes remain untouched, including +10, +2A..2B, +4D..4F, +64,
+66..67 and +79..7F. Name resize at B011B4 has RET8; the subsequent BF7680
copy at B011CC has ADD ESP,0C and reloads live source/destination fields after
allocation. The existing actual-header NativeString implementation supplies
the original storage behavior. BF7680's overlap behavior is represented by
the host `memmove` service; the seven-word record transfers use forward
REP MOVSD, including overlaps.

The initial 1Ch record is `{0,0,CE6650,CE6650,half[4],stack_word}`: four raw
float DWORDs, four 16-bit values and one DWORD never written by the native
constructor. B01274 calls the actual `D3DXFloat32To16Array` import with three
arguments and stdcall cleanup of 0Ch. Its thunk C2DFCE is `JMP [CE2410]`.
The new strict import resolves that named export from the caller's actual
loaded d3dx9_40 module; it does not reproduce library half conversion.
`initial_record_stack_word18` explicitly supplies the native incoming stack
word for exact replay. It has no known default, and is never synthesized as
zero or obtained by reading uninitialized C++ memory. Constructor consumers
must supply it or regard only that output word as unspecified.

Reserve requests clamp to at least one with signed comparisons, and allocate
`uint32(request)*1Ch` with native wrap. BF55BE is a library tail to BF681B
operator new; BF6989 is a library tail to BF65AC free. Both cdecl call sites
clean four bytes. Count is re-read during the seven-DWORD copy loop. A null
destination skips the current record copy but still advances its address;
there is no successful allocation-failure fallback. Old data is freed before
the new pointer/capacity are published. Count remains unchanged. All reserve
callers were checked: B00EE0 append, B00F30 clear, B00F70 destroy, B00FB0,
B01150 and B01350 load; append growth uses signed max(2*capacity,1).

## Destruction and table leaves

B00FB0 installs D5DDC0, then captures and disposes parameter pointers in order
+1C,+2C,+30,+48,+34,+38,+3C,+40,+44,+5C. Every nonnull pointer runs AFFDF0
then B00090 against the same captured slot. AFFDF0 frees +04 only for type16
at +0A equal to 1 or 2, and clears that payload after free. B00090 uses the
actual F8D344 generic 10h slot pool. Only owner+48 is cleared; the other owner
pointer fields remain stale. The descriptor path calls reserve(0) for signed
negative capacity, drains positive count, writes count=0 and frees its pointer
without clearing pointer or capacity. It next destroys the actual +08 name
through 419CC0/BD1510 and restores CEB130 through BD30F0. Scalar deletion
tests only flags bit0 and returns the captured pointer, even after free.

Native FH3 cleanup funclets CBB420/CBB428/CBB433 and
CBB450/CBB458/CBB463 confirm reference/name/descriptor cleanup roles; the
state-2 unwind order is descriptor, name, reference. These support this packet
and are not separately reconstructed or renamed here.

B00920 sequentially stores `+54=min_signed(old+54,uint32(count)-1)` then
`+50=min_signed(old+50,new+54)`; it has no lower clamp and count zero can
produce -1. B00740 is exactly RET4. The live D5DDC0 table has eleven DWORDs:
`BD30E0 B01130 BF698E B00920 BF698E BF698E BF698E BF698E B00740 BF698E BF698E`.
BF698E retains its correct library purecall identity. Table xrefs for the
leaves include other derived profiles; no unverified rendering behavior is
assigned to those slots.

## Required Ghidra repairs and verification

The worker performed no Ghidra writes. At capture, B00C20 has undisassembled
B00C95..B00C9E after incorrectly no-return BF6989; those ten bytes restore
ESP, publish pointer/capacity and restore saved registers. B00FB0 incorrectly
ends at B010EA; its real body continues through RET at B0112F, including
calls B01104->419CC0, B0110B->BD1510 and B0111A->BD30F0. B01130 also needs
the three-byte ADD ESP,4 gap at B01145..B01147 disassembled. B00920..B00940
and B00740..B00742 require leaf-function creation. The integrator owns repairs,
ledger annotation, refreshed exports and final call-row replay after release.

`C:/Users/sqz269/bsp-ap-base/capture.py` checked all six complete native byte
spans against live `/battlestationspacific.exe` in `C:/Users/sqz269/bsp.gpr`
and the installed PE. Its `byte_evidence.json` records the PE/span SHA256
values, last RET and direct calls. The installed PE has no relocation table;
the generator uses Capstone instruction operands for calls and absolute
addresses. It never scans arbitrary DWORDs for candidate addresses.

Strict MSVC Win32 `/O2 /EHsc /MD /fp:strict /W4 /WX` compilation passed.
`C:/Users/sqz269/bsp-ap-base/probe.cmd` compiled and ran the ad hoc
`base_probe.exe` with embedded manifest and explicit main return 0. It links
the current integrator's `build/win32/Release/bsp_core.lib`, and the new source.
Native differential checks passed with actual exit code 0: three constructor
and destructor comparisons (ordinary name, self-name, changed live scalar bits,
explicit stack residue, all ten parameter slots), three reserve comparisons,
eight scalar deletion executions, 36 signed range comparisons and RET4 stack
cleanup. The actual Windows SysWOW64 d3dx9_40 export performed conversion on
both sides. Original normal-path native bodies keep original FH3 handler
addresses unused; original exception execution was not attempted.

The report carries per-routine call-site rows, source hashes, native evidence
and the initial call verification result. No permanent tests were added.
Full project build and final current-library replay belong to integration.
This establishes reconstructed, strict compile-tested and native fixture-tested
behavior; it does not establish original ABI compatibility or game validation.
