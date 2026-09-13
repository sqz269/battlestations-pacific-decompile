# Native ambient construction

Address: 00B7C290.

`construct_native_ambient_00b7c290` reconstructs the complete normal body
**00B7C290..00B7C418, 393 bytes, 90 instructions** over the actual 98h ambient
allocation. This extends the existing typed-body evidence for
`BSP_AmbientLight_Construct`; it is not a newly discovered native body or a
complete ambient lifetime implementation. The descriptive name is a hypothesis.

| Routine | Coverage | Original ABI |
| --- | --- | --- |
| B7C290..B7C418 | complete | ECX actual owner, zero stack argument slots, EAX same owner, plain RET at B7C418 |

The new C++ interface borrows the actual owner and current four bytes at D7A24C.
It neither allocates nor creates `ConcreteSystemAmbientLight`'s appended host
lighting view. Raw storage must cover the original access domain; input pointer
values and private C++ locals remain stable. Pointed-to owner and constant
storage may overlap. No null guard, memset, typed owner overlay or copied
allocation preimage is added.

## Native producer and current constant

All three direct call sites and their allocation preparations were inspected
live. Each invokes BF681B with size98h, cleans four argument bytes after the
allocator, null-checks its result and passes the result in ECX to B7C290.
The constructor has no callees and does not perform allocation itself.

| Containing function | Allocation call | Constructor call | Original setup |
| --- | --- | --- | --- |
| B83C50 | B83CF2 -> BF681B | B83D09 -> B7C290 | PUSH98h; ADD ESP,4; MOV ECX,EAX; zero constructor arguments |
| 4C9EC0 | 4C9FA0 -> BF681B | 4C9FBA -> B7C290 | PUSH98h; ADD ESP,4; MOV ECX,EAX; zero constructor arguments |
| 93CCC0 | 93CF8C -> BF681B | 93CFA9 -> B7C290 | PUSH98h; ADD ESP,4; MOV ECX,EAX; zero constructor arguments |

Numeric callsite containment and callee starts are recorded in the report.
These caller slices establish the allocation/ABI domain, not complete parent
coverage. B83C50's whole raw3Ch inner lighting owner remains separate work.

The first memory input is MOVSS XMM1,[D7A24C] at B7C298, before the initial
owner store at B7C2A2. Live and installed-PE bytes are `00 00 80 3f` (float1.0),
but the implementation reads the current bits once. A different value, NaN,
signed zero or alias into owner storage is copied as-is. No floating arithmetic
or x87 operation occurs in this constructor.

## Exact effects and order

| Owner offset | Effect |
| --- | --- |
| +00 | store CEB130, then D62F3C; native vtable words are recorded, not made into callable host tables |
| +04 | reference DWORD1 |
| +08/+0C/+10 | zero native backlink pointer/count/capacity |
| +14 | captured current D7A24C bits |
| +18/+1C/+20 | positive-zero bits from XORPS XMM0,XMM0 |
| +24 | captured current bits |
| +28..37 | **untouched allocator preimage; no reads or writes added** |
| +38..97 | six four-word cube records; three positive-zero words and one captured-current word per record, in native instruction order |

The source retains the original local MOVSS stores and separate DWORD reloads
before owner stores. The last records include +78 before +74, followed by
interleaved local reloads and +7C/+80/+84/+88/+8C/+90/+94 stores. Replacing this
with aggregate initialization or an ascending loop changes the partial state
at an access fault. Explicit source-order verification accounts only for the
new ABI owner/constant pointer loads, C++ private spill addressing and frame
setup/return; all remaining 88 instructions match the live native sequence.

The existing typed constructor in `system_lighting_owners.cpp` creates an
additional host lighting view and uses host lifetime/size. Its native-shaped
prefix is useful prior evidence; it is not the raw98h allocation supplied here.
No duplicate owner type or changes to that shared implementation are introduced.
Construction alone does not prove raw backlink updates, ambient destruction,
scene attachment, current virtual dispatch or complete inner scene construction.

## Verification

Every live `bsp.py ghidra` batch verifies the configured BSP project/program:
C:/Users/sqz269/bsp.gpr, /battlestationspacific.exe, x86:LE:32:default,
image base00400000. All393 body bytes and the four constant bytes match the
installed PE. The complete body was exported read-only. The worker made no
Ghidra annotations, prototype changes, definitions, saves or ledger edits.

Strict source compilation uses MSVC Win32 /O2 /W4 /WX /fp:strict. The emitted
assembly was inspected for the single captured constant, the local spill
sequence, nonascending owner writes and lack of accesses to +28..37.
All eight `verify-seeds` rows matched; the baseline Win32 build and both existing
CTests passed at commit `e109c2182d6e59aa1d5a4206fa756afbf9af534c`.
The six numeric allocation/constructor call rows passed the live containment
and callee audit. The baseline excludes this unregistered source;
central CMake registration and a current-library-only replay remain necessary.

The one focused external fixture preserves the unchanged native393-byte body
in executable memory and the original absolute D7A24C operand in a disposable
process. It compares complete borrowed arenas, return pointer, MXCSR, and
bounded access-fault codes/partial memory states. Cases cover eight current
constant bit patterns, three MXCSR modes, all38 owner-word constant aliases,
a separate constant, eleven guarded write frontiers and an inaccessible
constant before the first owner store. Private stack/local addresses and native
fault instruction pointers are outside the comparison.

The fixture passed **948 original-byte comparisons**, comparing 61,390,848
arena bytes in total. The initial run explicitly compiled this unregistered
source with the baseline libraries. Source, header, probe, include, recipe and
all three libraries had identical hashes before compilation and after the run.
The original executable body was unchanged after all comparisons.

The immutable worker archive is
`C:/Users/sqz269/bsp-ba-ambient-construction/worker_capture.zip`, containing
source/header, original spans/exports, probe/scripts/includes, three baseline
libraries, hashes and complete logs. `run.ps1` defaults to compiling only the
external `probe.cpp` against the current three libraries; `-WorkerSource` is
only for the initial unregistered worker stage. Probe linking embeds a manifest.
All 47 archived members were read back and verified against their hashes.

These checks do not establish original caller ABI compatibility, native EH/SEH
unwind identity, races, complete ambient lifetime, parent scene behavior or
gameplay. No new repository tests, CMake changes or shared-file edits were added.

## BA integration checkpoint

The integrator reviewed the complete native body and actual producer evidence,
saved its original signature and complete stored range in the existing BSP
project, and registered the source. Current combined validation follows
separately from the source or worker checks above. No complete owner lifetime,
original binary replacement or gameplay claim follows from this checkpoint.

## BA exact merged validation

The exact combined source commit `b852ae06a7fdd93c799cdacc015a1a5a96adf9f1` passed the strict Win32
build and both existing tests. Four current-library-only original-byte fixtures
cover the ambient, registry and two vector modules; the saved-dimension leaves
have exact complete emitted-byte checks in the built library, with no runtime
fixture added. See `reports/native_lighting_service_ba_validation.json` for
hashes, immutable captures, coverage and limits. Earlier pending statements
describe worker stages. Native ABI, full rendering and gameplay remain open.
