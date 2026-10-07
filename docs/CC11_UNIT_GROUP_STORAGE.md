# CC11 actual unit-group storage constructor

`0070DAB0..0070DB1B` is a complete 108-byte, 35-instruction constructor with
no calls. The reconstruction in `src/native_unit_group_storage.cpp` initializes
the actual 0x508-byte receiver layout. It takes borrowed writable, aligned
Win32 storage and the actual `00CF4888` constant bits through a new C++
interface. Native ECX carries the receiver, native EAX returns the same address,
and the original ends with plain RET. This source interface is not the native
class ABI or a C++ polymorphic object.

## Exact initialization

`0070DAB0` captures the four bytes at `00CF4888` before any receiver store.
The live analysis, installed PE and frozen PE agree on `00 C0 79 44`, the
binary32 representation `4479C000` of 999.0. The source captures these bits once;
it does not calculate a float or call a default-speed provider.

The receiver writes occur in native order:

1. Zero the words at `+04`, `+08`, `+0C`, then only the byte at `+10`.
2. Store the raw `00CFD6F8` profile word at `+00`.
3. Visit all 24 records at `+18 + index*34h`. Zero record `+00`, store the
   captured speed bits at `+30`, then zero four columns. In each column the
   axial word at `+20 + column*4` precedes the lateral word at `+10 + column*4`.
4. Zero group `+14`, then `+500`, then `+4F8`, and return the receiver.

The profile store is at **0070DACE**. The older ledger's `0070DAE1`
instruction attribution is incorrect; primary-owned metadata must use DACE.
The copied profile is deliberately **uncallable raw data** in this interface.
No virtual table or substitute class implementation is supplied.

Exactly 989 receiver bytes are initialized. The remaining 299 are preserved:

| Preserved region | Bytes |
| --- | ---: |
| Header padding `+11..+13` | 3 |
| Each record's position triplet `+04..+0F`, 24 records | 288 |
| Group `+4FC..+4FF` and `+504..+507` | 8 |

There is no aggregate clear, added null guard, allocation, publication, callback
or ownership operation. Valid native storage and the genuine readable constant
cell are the domain. Volatile field accesses preserve capture and store order
on the MSVC Win32 target; they do not establish concurrent synchronization.

## Actual producer and consumer evidence

The original factory `0070DB20` pushes allocation size `508h` at `0070DB22`,
calls the allocator at `0070DB29`, and calls this constructor at `0070DB37`.
It then calls the real `0070D7B0` publisher at `0070DB41`. That publisher writes
group `+14` and the leader unit's `+284`, then reaches additional real work.
These pinned ranges establish the receiver and field lineage; the factory and
publisher are not reconstructed by this packet.

The actual dispatcher arm calls the already reconstructed owner predicate at
`007805A0`, tests AL, and later calls `0070EFD0` at `007805B7`. Formation still
depends on the unclosed `00424C40` settings singleton path, its constructor and
Lua loading. This standalone constructor closes one genuine storage producer;
it does not close formation or the full `00780120` dispatcher. Existing semantic
formation code remains unchanged.

## Focused differential evidence

`reports/cc11_unit_group_storage.json` pins the complete native and source
evidence. Artifacts are under `local/cc11_unit_group_storage_20261007_a`.
Ghidra remained read-only, using `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe`.

One focused fixture executes the complete original 108 bytes against canary
storage, then the source against an identical receiver. The only original-code
relocation is its existing four-byte absolute constant operand at body `+04`.
It points directly into the read-only frozen original PE mapping at the actual
`CF4888` cell. All other 104 original bytes remain identical, including the
profile immediate. There are no dependency CALL bridges or repository support
libraries. Code and constant checks pass before and after execution.

The original and source agree across the whole 1,288-byte receiver and both
16-byte guards. An independent per-byte field map confirms all 989 initialized
values and all 299 preserved values; both routines return their actual receiver.
The single fixture passes **32 checks with zero failures**. It starts with
nonzero canaries, so an unwanted aggregate clear cannot pass the comparison.

The fresh build uses two translation units: current source and fixture. It
compiles with `/O2 /W4 /WX /fp:strict /showIncludes`, links an embedded
`asInvoker` manifest, and consumes four frozen repository/fixture inputs.
All actual includes match that snapshot; 173 host-header hashes and six searched
toolchain-library hashes are recorded. No BSP library is linked.

The complete Source COFF body is 132 bytes and 39 instructions, with no calls
or relocations. Its load at `+0C` captures the constant before the first receiver
write at `+0F`; header writes occupy `+0F..+28`. The outer loop covers 24 records;
the inner loop covers four columns. Axial `+50` precedes lateral `+5B`, whose
address includes the preceding index increment. Final writes at `+70`, `+75`
and `+7C` target `+14`, `+500` and `+4F8` in that order. The entire 20-byte,
10-instruction original-entry adapter is also recorded.

All seven earlier worker reports and their 814 referenced artifacts retain
their hashes. Their fixtures were not replayed. The installed original PE and
its fresh frozen copy retain their hashes as well.

## Qualification

This is reconstructed, strict-Win32-build-tested, and focused-original-fixture-
tested source for the complete constructor. It is not a native class-ABI drop-in,
allocator failure or exception-unwinding proof, callable native profile,
publisher/formation/settings closure, or game validation. Shared registration,
metadata correction and the full project build remain primary-owned.

Primary integration at `d2d2fa396457f95ac46bb709f4a62fc9adf1c7b4` passed the full MSVC Win32 build and all three existing CTests. Independent primary validation freshly compiled2 actual TUs,3 Source/header/fixture pins,2 actual project compiler includes, the current linked BSP libraries (none for group storage), original PE and860 historical worker inputs. Complete Source COFF bytes match the worker. Whole108B35inst0070DAB0 actual508h storage; nativeCF4888 bits4479C000 capturedONCE beforestores. Header4/8/Czero/ONLYbyte10zero/profileCFD6F8 at70DACE, correctingold70DAE1 attribution.24x34h records18: entity0/speed30/eachaxial20..2C beforelateral10..1C; final14→500→4F8 zero, actualreceiverreturn. Exactly989initialized/299preserved (positions/padding/4FC/504), no aggregateclear/defaultspeed/allocation/class table. NewSourceactualconstcell reference+validalignedWin32storage domain. Onewhole508h+32guard Source-original fixture32checks; original108B ONLYnatural4Bconstantoperand relocatedtoactualreadonlyfrozenPEcell/104otherbytesretained inclrawprofile. Source132B39inst0calls/relocs:constcapture0C precedesfirststore0F;axial50 beforelateral5B/final70,75,7C. NoBSPlibs/providerbridges;70DB37 caller/508hallocation/publisher lineage structuralonly. Fullgroup/classABI/nativefactory/publishervirtual5C/formation/settings/Lua/failure/EH/gameunbound.
