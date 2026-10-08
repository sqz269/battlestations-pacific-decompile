# Type-6 constructor and empty-child clone Source readiness

Two bounded interfaces are implementation-ready after primary acceptance of producer commit `cf8dac008a3c81d62f1194a4e622d118e306163f`: the **whole 58-byte `008EF780` raw constructor** and an **ordinary C++ successful-empty-child fragment for the complete selected 71-byte type-6 arm `[008F50ED,008F5134)`**. The existing ordinary empty-bag clone supplies the actual distinct child needed by the constructor. This packet implements neither interface, changes no Source or metadata, and awards no function/fragment credit.

The current producer was read and physically captured from `J:/PROG/battlestations-pacific-decompile-cc12_resume_integrator` after its current normal build. Its complete ordinary clone is 126 bytes/41 instructions; whole raw61 is byte-identical to the installed native constructor. The canonical allocator/free are 90/6 bytes. All three selected complete objects, including the physical-pool process context, occur exactly once and byte-for-byte in the copied whole archive. This is current Source/production evidence, without new API or Original execution.

## Actual input domain and interfaces

The proposed new, disjoint files are `include/bsp/native_scene_property_record_type6_storage.hpp` and `src/native_scene_property_record_type6_storage.cpp`. These signatures describe raw memory, not a recovered C++ class:

```cpp
void* __fastcall construct_native_scene_property_record_type6_storage_008ef780(
    void* actual_record_ecx, std::uint32_t unused_edx,
    void* actual_child_bag) noexcept;

void* clone_empty_child_native_scene_property_record_type6_008f50ed_fragment(
    const void* actual_source_record);
```

The raw constructor takes a real fresh writable `38h` allocation from `singleton_lifetime_allocate({object, 0x38, 0x38})` and the distinct, actual mutable `114h` output of `clone_empty_native_scene_property_bag_008f41f0_fragment`. The latter is called only with its established stable, valid raw61-produced source domain: count `+8=0`, all 64 heads zero, DF clear, no aliasing or concurrent mutation. The child has owner `+110=0` before attachment. Both allocations belong to the genuine same current malloc/free domain. Neither a stack dummy record nor a fabricated bag/header/profile establishes this admitted owning-output route.

The ordinary arm input is a live raw record actually produced by this whole constructor, with source tag `+4=6`, actual child at `+C`, and initialized ordinal at `+34`. That child remains in the qualified empty-source domain during the operation; its owner backlink to the source record is valid and is not copied to the new child. Source root, source child, new record, new child, and active call frame are distinct. Source roots remain borrowed. The fragment is explicitly selected for type 6; it is not a replacement for the whole tag-dispatching `008F4F60` entry or a runtime tag/profile dispatcher.

This route does not require an E175B0 node allocation or a source map scan: the admitted child contains no nodes. The now-real, distinct E175B0 cell on the canonical E188B4 process is relevant to later populated-map ownership. Its presence is not substituted for either malloc-domain root.

## Whole constructor: exact ABI, stores and preservation

The complete installed/live body is `[008EF780,008EF7BA)`, 58 bytes/16 instructions, SHA256 `6ecd8cb72e2e8d06bf1f3f5c53f442d88c3b3606ffc3dacdc6a98f9c5a2f0450`. It is call-free and requires no unavailable profile dispatcher. Ghidra's printed `void __thiscall` return is incomplete: assembly returns the record in EAX.

At entry ECX is the actual record and `[ESP+4]` is the actual child. Incoming EDX is ignored. The explicit unused second fastcall formal is necessary to keep the child on the stack; two fastcall formals would wrongly place it in EDX. `RET4` consumes exactly one argument; returned ESP is entry ESP+8. EAX is the same record, EDX is the child, ECX is zero. EBX/ESI/EDI/EBP and DF are unchanged. Final arithmetic flags are those of `XOR ECX,ECX`: CF=OF=SF=0, ZF=PF=1, AF undefined. No x87, SSE or MXCSR operation occurs.

| Native instruction | Exact operation |
| --- | --- |
| `008EF780` | Read stack child into EDX, before any record store |
| `008EF784`, `008EF786` | EAX=record; ECX=0 |
| `008EF788`, `008EF78E` | Store literal phase `CE89D4` at `+0`, tag 6 at `+4` |
| `008EF795` | Store actual child pointer at record `+C` |
| `008EF798..008EF7A4` | Zero record `+18,+1C,+20,+24,+30`, in that order |
| `008EF7A7` | Store actual record pointer at **child `+110`** |
| `008EF7AD` | Zero record ordinal `+34` |
| `008EF7B0` | Store byte 1 at record `+2C` |
| `008EF7B4`, `008EF7B7` | Zero record `+8`; `RET4` |

Exactly 41 record bytes are written: `[00,10)`, `[18,28)`, `[2C,2D)`, `[30,38)`. Exactly 15 are preserved: `[10,18)`, `[28,2C)`, `[2D,30)`. There is also the distinct four-byte child backlink write. Do not memset all 56 bytes, clear preserved fields, move the backlink after the ordinal, or add a weak null-only constructor branch. The supplied child is dereferenced unconditionally. An MSVC Win32 naked body following the existing raw-storage pattern can preserve all 58 bytes and this physical ABI; the ordinary arm has a separate C++ ABI.

## Successful type-6 arm: actual transport and Source sequence

The complete selected arm is 71 bytes/21 instructions, SHA256 `e13774b4eac8a8c1f666445c908ad604d586777101dac0eec941639a0d0d44c6`. The verified 45-byte parent prefix reads source `+4`, compares it unsigned to `0B`, and jumps via `[008F52B8+tag*4]`. Actual table cell `008F52D0` contains `008F50ED`. The constructor call is **`008F5118 ->008EF780`**; `008F5115` is the preceding `PUSH EAX`.

Let S be ESP at the native parent `008F4F60` entry. At the selected arm ESP=S-24, ESI=source, `[S-24]` is saved EDI, `[S-20]` saved ESI, `[S-16]` the scratch record slot, `[S-12]` old FS link, `[S-8]` handler `CA4B6D`, and `[S-4]` unwind state. These are inherited parent-frame requirements, not independent callable-arm arguments.

| Instruction(s) | Native transport | Ordinary fragment operation |
| --- | --- | --- |
| `50ED`, `50EF` | PUSH38h; CDECL call BF681B with size at callee ESP+4 | Request `{object, 0x38, 0x38}` from actual current allocator |
| `50F4..50F9` | EDI=EAX; ADD ESP4; save EDI at `[S-16]` | Retain the real fresh record pointer |
| `50FD..5107` | TEST EDI; set state2 without changing flags; JZ `008F528A` | Admit normal nonnull allocation only; native null/EH tail is outside scope |
| `510D`, `5110` | ECX=`[ESI+C]`; call whole native bag clone, no stack args | Read actual source child and call the current ordinary **empty-source** clone |
| `5115..5118` | PUSH returned child EAX; ECX=EDI; call whole58 | Pass that exact returned child to the whole raw constructor |
| `511D..5121` | EDX=`[ESI+34]`; POP EDI; write `[EAX+34]=EDX` | Re-read source ordinal **after** construction; copy its raw DWORD |
| `5124..5133` | POP ESI; restore FS:[0]; ADD ESP10h; RET | Return the actual new record through ordinary C++ return |

The bag call enters with ESP=S-28 and plain RET restores S-24. The child push then makes S-28; constructor entry is S-32 with its child at S-28. `RET4` restores S-24 before the observed POP EDI. After the native epilogue ESP=S+4, EAX=new record, EDX=source ordinal, ECX=old FS link, and ESI/EDI are restored. Final arithmetic flags come from `ADD ESP,10h`, not the constructor's XOR. The ordinary C++ fragment does not claim these whole-parent register/FS/SEH effects.

The complete body operation is therefore allocation38, load source child, actual empty-bag clone114, whole constructor/backlink, load/copy source ordinal, return. The current bag Source performs real allocation and raw61 initialization **before** reading source count, matching the native empty path; its raw61 stores owner `+110` before ordinal `+10C`. The native whole clone's zero-count iterator exit has no node/head scan, child recursion, insertion or advance. A nonempty input cannot be silently reduced to empty.

Keep the ordinary function non-noexcept because genuine allocation can throw. If an exception occurs before child attachment, releasing only its fresh unattached record allocation through `singleton_lifetime_free` is permissible Source error cleanup. That is not native CA4B6D/state2 cleanup or a fabricated record destructor. The existing bag clone already frees its fresh empty output before its Source-only nonzero-count rejection. The admitted native behavior remains normal successful allocations and valid memory. After child production the raw constructor and ordinal transfer have no C++ throw on that domain. No recursive cleanup callback, fake RAII owner, or new failure/SEH claim is needed.

## Real profiles and allocator boundaries

The retained installed/live DATA words are `CE89D4 ->004E6730`, `D16504 ->008F59E0`, and `D162C4 ->008F4170`. Constructor and clone store these literal identities; neither requested interface calls through them. They are not readable/callable Source vtables. The nested future type-6 release's actual flags1 slot0 call selects the bag scalar only on this proven producer domain. Inner-map scalar4170 is outside the six-body owning-clear cycle.

Installed whole operator-new `[00BF681B,00BF6884)` is 105 bytes/33 instructions; its normal path calls native malloc `00BF9F1A`, retries through `_callnewh` `00C055B1` on null, and otherwise returns its pointer. Failure construction/throw and historical CRT state are retained evidence, without admission. Whole `00BF65AC` is a five-byte tail jump to native free `00BF9DC8`. These are native CRT services, not interchangeable function addresses in the Source process.

Current full allocator COFF actually imports `__imp__malloc` and `__imp___callnewh`; full free is a six-byte jump through `__imp__free`. The captured current application imports all three through `api-ms-win-crt-heap-l1-1-0.dll`. The copied installed I386 `ucrtbase.dll` exports their concrete roles. The Source empty-clone body calls the actual allocator and raw61, has the real security-cookie/check dependency, and retains its cold free/`std::invalid_argument`/throw path. The allocator retains its real `std::bad_alloc`/throw dependencies. No Original CRT exception/vtable/global-handler identity, cold path execution, or runtime API-set-resolution witness is inferred.

## Integration and verification gate

The normal build is MSVC Win32 through `scripts/build.ps1`; it passes `cmake/startup.cmake` as `CMAKE_PROJECT_INCLUDE`. Current bag Source already has an explicit deferred target registration. Source files are not globbed, so merely adding the disjoint type-6 files does not compile them. Root must coordinate this single registration with the external owner of that file:

```cmake
cmake_language(DEFER CALL target_sources bsp_core PRIVATE src/native_scene_property_record_type6_storage.cpp)
```

This packet does not edit that file, another current Source TU, or build configuration. After authorization and registration, run the normal build and its existing checks. Qualify the complete emitted constructor against all 58 installed bytes, complete ordinary fragment plus ordered relocations, genuine allocator/child calls, complete selected objects and unique archive members. Constructor/arm runtime and failure/EH proof remain separate. No new tests or probe framework is proposed.

The primary current build receipt reports all three existing checks passing in 10.16 seconds; those are reused producer-build results, not a type-6 build/test run. The evidence collector ran after that build. The four producer Source/header files and two build-recipe files match the separately retained pre-build manifest. This worker's copies are honestly post-build; parent primary acceptance remains the authorization gate.

## Exact remaining lifetime family

Creation does not close recursive destruction. The existing real allocations remove the old child-producer gap, and the proposed actual record producer removes its own gap only after implementation and qualification. The following **six whole bodies must be handled cohesively**, without another tiny-scalar deferral:

| Body | Required whole behavior and dependency |
| --- | --- |
| `008F3F30` (254 bytes) | Clear actual 64-head owning map; retain next before mapped release; invoke actual record flags1 scalar; then mapped-zero, real raw key return, same E175B0 node/page return, head clear, final count clear |
| `004E6730` | Whole record scalar: call `008F0DE0`, conditionally free the same genuine record root according to low flags bit, retain full stack/return behavior |
| `008F0DE0` (11 bytes) | Publish literal record phase CE89D4, tail-transfer unchanged receiver to whole release `008F0640` |
| `008F0640` (158 bytes) | Preserve complete tag dispatch and CString/array/free/reset schedule; nonnull tag6 must invoke actual child's scalar with flags1 before payload clearing |
| `008F59E0` (30 bytes) | Whole bag scalar: always call `008F5410`, conditionally free actual root on low flags bit, return its original address even after free, RET4 |
| `008F5410` (88 bytes) | Whole ordinary bag destruction: outer phase, same embedded map clear, inner D162C4 phase, same-map clear again; no interior/root free |

The cycle is `3F30 ->4E6730 ->0DE0 ->0640(type6) ->59E0 ->5410 ->3F30`. Required concrete Source code-domain bindings are producer-qualified CE89D4/slot4E6730 records and D16504/slot59E0 bags to those actual whole compiled services. No synthetic profile memory, arbitrary destructor callback, unknown-profile default, semantic-class reinterpretation or null-only whole-family claim is allowed.

Its live inputs must be finite unshared acyclic owning allocations, genuine same-domain nodes/page IDs and raw keys, stable actual mapped records, and a real process/string/pool context lasting through destruction. Finish these borrowers before actual E175B0 teardown. Existing opaque mapped-DWORD publication still does not supply wrapper record owner `+30`/ordinal or whole replacement `008F28F0`/insertion `008F33F0`; whole populated clone, recursive factory, original public class ABI and Original EH remain outside this ready packet.

## Evidence scope

`reports/cc12_property_type6_source_readiness.json` carries original instruction/transfer rows, exact typed/raw contracts, the complete dependency inventory, current whole-object/archive qualification and final seal. Ignored `local/cc12_property_type6_source_readiness/` physically retains selected Source, headers, prior evidence, complete objects, unique members, whole archive/application/UCRT, exact native spans, live captures and the actual capture/decoder tools used. The entire installed PE is identity-pinned; selected full spans are copied, not the full executable.

Each live analysis used the standard `bsp.py` verification of project `bsp`, program `/battlestationspacific.exe`, x86 language and base; retained configuration pins existing `C:/Users/sqz269/bsp.gpr`. No Ghidra write, export/ledger change, new Source/API execution, new build, test, probe, game launch or gameplay validation occurred. Older tag-6 mapping and payload-binding reports describe historical leases/missing producers; their old no-Source-ready statements do not override this freshly qualified callee contract.

## Primary review

Independent primary review verified all249 retained artifacts,11 installed spans,138 complete current functions and three unique archive members. For the next Source packet, the approved compilation route is the already registered `src/native_scene_property_bag_storage.cpp` plus the disjoint `include/bsp/native_scene_property_record_type6_storage.hpp`; preserve its15 existing extents. This supersedes the earlier separate-TU/CMake proposal and leaves the externally leased build file untouched. The raw58 constructor and successful empty-child arm are ready; the six-body recursive destruction family remains separate. No new Source, build, execution or Original credit occurred.

Receipt: `local/cc12_read_and_type6_readiness_primary_review/receipt.json`, SHA256 `ca1d7af3fb75be1418c524c2aca250fbf9e6709dbde31ff1f4710461378557e3`.
