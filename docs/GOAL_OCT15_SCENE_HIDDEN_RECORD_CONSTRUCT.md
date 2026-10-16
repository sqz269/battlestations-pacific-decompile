# Hidden record constructor code with bounded provider admission

Addresses: `004693C0`, `00C61AD0`. Packet `goal_oct15_scene_hidden_record_construct`.

**General Source-ready remains false.** This packet covers the whole native code
schedule using real providers. It inherits the duplicate provider's bounded
normal current-allocation admission, not unrestricted failure, new-handler/reentry,
private unwind, class ownership or Original FH3 equivalence. It does not establish
general raw closure of the generation gate or admit its other missing services.

| Routine | Code coverage | Native ABI | Ordinary Source |
| --- | --- | --- | --- |
| `BSP_SceneHiddenEntityRecord_Construct` | Complete178B/56ops, `004693C0..00469471` | ECX record,six LIVE stack words,RET18h,EAX captured record | `construct_native_scene_hidden_entity_record_004693c0` |
| `Unwind@00c61ad0` | Qualified complete11B/3op action, `00C61AD0..00C61ADA` | CURRENT native[EBP-10h],add54,tailJMP pooled destructor | `unwind_native_scene_hidden_entity_record_00c61ad0` |

Owned code totals189 bytes/59 instructions. Native compiler/library names and
prior comments remain preserved. Descriptive names are hypotheses. The existing
`seg6_cc13.cpp` implementation uses Seg6Host services; it is retained separately.

## Actual storage and argument schedule

The record's92 bytes contain the original image word00CE5640 at0, opaque bag4,
duplicated name8/classC, raw party10,64-byte matrix14..53, and8-byte parent header
54/58. Writing the borrowed image word is not a Source vtable, callable binding
or C++ object-lifetime claim. No nominal owner/mapped class is introduced.

The explicit incoming24 cells are bag,class,name,party,matrix,parent header at
offsets0,4,8,C,10,14. Metadata is stable/disjoint from supplied storage. The
distinct saved-this4 binding represents the actual private cell, not another
native argument. Supplied payload/record may alias both kinds of cells; implicit
compiler/native private-state and outgoing-argument homes are not exposed.

Preserve the initial PUSH ECX saved-this store, vtable store, repeated saved-this
store and parent54/58 zeros. Capture current bag and name before arming cleanup
state0 and publishing bag. Duplicate name through the actual provider. Capture
current class pointer before storing name8, then duplicate class. Capture current
party and matrix before publishing party10 then classC. Pass the captured matrix
pointer to the exact raw x87 provider with destination record+14; do not snapshot
or reinterpret its floating payload.

Only after matrix copying, capture current parent-header pointer. Equal own54
skips all source reads and resizing. Otherwise read current parent length, invoke
actual raw resize with preserve1, then reread parent length. When nonzero, read
current destination length, source data, destination data in that order. Copy
DESTINATION length with the real current SDK overlap-capable memmove. Retain the
zero-count call after those reads through a Source-only naked tail adapter to
the actual SDK symbol; no C++ memcpy overlap restriction or zero-null builtin
assumption is introduced. The adapter has no Native reconstruction credit.
The existing primary executable imports memmove from VCRUNTIME140.dll. The
installed x86 C:/Windows/SysWOW64/VCRUNTIME140.dll export at RVA161A0 was inspected:
count0 always reaches1669B, branches to166D0 and returns the destination without
source/destination memory reads. Its file hash is recorded in the report. This is
an installed-provider static check, not proof of a newly loaded process binding;
the emitted adapter is five bytes E9 rel32 to external _memmove. The exact
Win32 vcruntime.lib import member binds that symbol to VCRUNTIME140.dll.
The constructor and adapter are compiled and archived; their unreferenced
body is omitted from bsp_game.exe, so no new linked-entry or execution
credit is claimed. No forced link or new test was added.

## Sole cleanup action

Native state0 becomes active after bag/name capture and before the first
duplicate. FuncInfo00D89120 points to the one-entry map00D89118: state0 unwinds to
-1 through action00C61AD0. That action reads CURRENT saved-this4, adds54 modulo32
and reaches the actual pooled string destructor. It does not free either raw
name, the bag or the record. Normal completion disarms the Source guard and
returns the original captured receiver, independent of the current saved-this
cell. No cleanup is added before state0 or after a normal return.

The Source guard supplies current C++ cleanup transport. Original handler
00C61ADB/FH3, private state corruption, SEH, secondary cleanup throws and original
naked-frame failure transport are not equivalence claims. Exposing saved-this4
does not expose other implicit compiler/private homes or promote those domains.

## Actual native transfers and callers

| Actual listing | Containing function | Contract |
| --- | --- | --- |
| `00469408 CALL 00438E40` | `004693C0` | captured current name pointer; no stack arguments; plain RET/EAX copy |
| `00469414 CALL 00438E40` | `004693C0` | class captured before name8 store; no stack arguments |
| `0046942B CALL 004134F0` | `004693C0` | ECX record14,one captured matrix pointer,RET4 |
| `0046943F CALL 0041DD40` | `004693C0` | ECX own54,push preserve1 then current parent length,RET8 |
| `00469454 CALL 00BF7680` | `004693C0` | push DESTlen,srcdata,destdata;ADD ESP,0Ch |
| `00C61AD6 JMP 0041DD20` | `00C61AD0` | sole current saved-this+54 cleanup action |
| `0046C62B CALL 004693C0` | `0046C550` | six stack words,cloned bag pushed last,RET18h |
| `0046CBDD CALL 004693C0` | `0046C550` | second six-word constructor site,RET18h |
| `0046D6F4 CALL 004693C0` | `0046CF40` | six words including actual local matrix/parent header,RET18h |

All three incoming setups and full owned listings were read. Native memory copy
targets00BF7680; Source current-memmove adapter is a qualified service binding,
not a false Native transfer edge.
The cleanup opcode is an actual E9 JMP; Ghidra retains its existing CALL_RETURN
flow override (default unconditional jump, effective call terminator). Both typed
views and targets are preserved; no flow or prototype repair is applied.
The actual matrix provider is already complete
103B/35ops at004134F0..00413556 (3557 exclusive; following CC is alignment).
Fresh current COFF bytes, zero relocations, installed PE and live bytes match
SHA25653914f4a6160375e4674eafefab73925952464b1edc62bd48edfa56c19068f10.
Its16 ordered FLD/FSTP pairs retain reached overlap, unaligned accesses and ambient
CW/status/NaN operations, with no finite guard or environment reset. Physical FP
instruction/fault addresses, SEH and runtime/native differential remain unproved.

## Provider admission and verification

The actual raw resize/destructor use NativeStringRawPoolContext and concrete
pool services, with live headers and current-field rereads. No Host conversion
or callback is added. Duplicate00438E40 is the existing literal57B current raw
entry with real canonical allocation and standard copy. Its published domain is
null or live readable contiguous nonwrapping NUL text; normal successful current
allocation produces a disjoint copy. The recorded exclusions include forced
allocation failure, new-handler/reentry, naked-frame unwind and Original private
CRT/EH. Code preserves service return/throw routes; this packet cannot enlarge
that admission or grant general Source-ready credit to constructor consumers.

Guarded analysis/export verifies project C:/Users/sqz269/bsp.gpr and program
/battlestationspacific.exe. Locked annotation preserves prior names/comments,
records prior values, saves, refreshes owned exports and reads back evidence.
The report holds exact189 owned original bytes, typed9transfer rows and provider
hashes. Primary independent review accepted the complete constructor and
qualified cleanup action at e2a64dac7 without Source changes. Both reviewed
Source hashes were asserted unchanged before recording acceptance. The
primary registered this Source once and ran scripts/build.ps1 at
1b43f15d71011cf545a6323c7b0951e86463d5fe: MSVC Release Win32 passed,
with all three existing checks passing in 7.96 seconds. The primary log
and receipt hashes are pinned in the report; no worker build is claimed.
The emitted Win32 object occurs exactly once in the tested archive.
Reviewed, built and provider Source hashes remain unchanged. No new
routine tests, direct native-entry execution, binary ABI, application or
game validation is claimed. General Source-ready remains false.
