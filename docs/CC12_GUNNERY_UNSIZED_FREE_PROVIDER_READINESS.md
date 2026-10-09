# CC12 gunnery unsized-free provider readiness

Primary acceptance: Accepted as147-byte/49-instruction/11-call static evidence:5-byte thunk,133-byte/44-instruction main partition and separate9-byte/4-instruction helper. Root replayed the original bytes, every physical operation and retained Source pins. Exact serialized Ghidra AddressSet was unavailable and is not inferred as a contiguous function. The current-pointer reload after unlock and errno-accessor-before-GetLastError order remain required. Helper code is ready for a separately reviewed Source candidate; actual allocator/node/lock-acquisition/frame ownership and full destructor remain held.

No Source, Original-ABI, startup or gameplay credit is added by this readiness review. Worker document/input pins retain their immutable capture meaning; current primary document identity is recorded separately.

The complete `00BF65AC` free entry and its target are understood at the byte/instruction boundary. A new free-dispatcher or gunnery-destructor Source implementation remains held: the actual node allocation domain, allocator selector/heap, descriptor lookup, lock acquisition, error services and SEH owner are not supplied by this packet. The separate nine-byte unlock helper is a concrete candidate for the next bounded Source leaf.

## Scope and evidence

This read-only packet owns Native `00BF65AC`, the explicitly approved target `00BF9DC8`, the separately approved contained helper `00BF9E1E`, and these two readiness files. Base is `1172ce41243d31a7f27313797f39b8600f1cea90`. Verified CLI queries use project `bsp`, `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, image base `00400000`. Original PE SHA-256: `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

| Unit | Physical bytes / instructions | Evidence |
|---|---:|---|
| `00BF65AC..00BF65B0` | 5 / 1 | Exact `JMP 00BF9DC8`; PE/live bytes and saved/live start agree |
| Main ranges `00BF9DC8..00BF9E1D`, `00BF9E27..00BF9E55` | 133 / 44 | Main partition agrees with live 44-instruction metadata |
| Separately defined `00BF9E1E..00BF9E26` | 9 / 4 | `PUSH 4; CALL 00C11B31; POP ECX; RET`; independent live function |
| Unique total | 147 / 49 | Every byte and physical instruction start checked |

The target's min/max span is 142 bytes and the saved/live listing prints 48 instructions because the separately defined helper occupies its interior. The typed metadata exposes min/max plus instruction count, not the serialized Ghidra AddressSet. The two main physical ranges exclude the helper and total the metadata's 44 instructions; this qualification is retained. Do not flatten the enclosing function into a contiguous 142-byte definition. All definitions and annotations are unchanged.

Target-span SHA-256: `d5bec1353d11880b9ac19e7aa0648037b7000906a0c9ba7754084429d0735955`. Thunk SHA-256: `901f036d0ec6475be7f50c9243861104afeab65061759b3109d41419c26ae979`. The JSON retains all 49 decoded operations, 11 physical calls (8 direct main, 2 indirect main, 1 helper), pins, provider comparisons and explicit limits.

## Calling and allocation behavior

The entry is a frame-preserving tail jump to a one-pointer `void __cdecl free(void*)`. The target pushes local size `0xC` and table address `00E02DF0`, calls the unopened `00C07C00` prolog, then captures the pointer from `[EBP+8]` into ESI. Every normal route reaches the `00C07C45` epilog and plain `RET`; the caller removes its pointer word. The signature has no meaningful return value. Saved-register/FS restoration depends on the actual compiler frame helpers; this is not a new original-ABI admission.

Null input still traverses prolog/epilog, but skips allocator mode/heap reads, locking, descriptor lookup, payload release and error publication.

For non-null input, the body reads `[0109ED7C]`. A value other than 3 takes the heap route using captured ESI. Mode 3 calls `00C11C21(4)`, clears `[EBP-4]`, calls the actual immediate entry `00C11D3D(payload)`, and saves the result at `[EBP-1C]`. The decompiler's `LIBCRT_unmatched_00c11d61` label does not prove the unopened `C11D3D` entry's body or target. A nonzero descriptor invokes `00C11D68(descriptor,payload)`; zero skips that call.

Both mode-3 routes then set `[EBP-4]` to `-2`, call local `00BF9E1E`, and test the saved descriptor. Nonzero returns without a heap call. Zero reloads the **current** pointer argument `[EBP+8]` after lookup/free/unlock, rather than reusing ESI, then enters the heap route. Preserve this reload in any future Source translation.

The heap route pushes flags 0 and the **current** `[0109E1BC]` heap, then calls through `[00CE20FC]`, labelled `HeapFree` by the existing decompilation. A nonzero result returns. Zero calls `00BFFB8B` first, captures its returned address in ESI, then calls `[00CE2258]`, labelled `GetLastError`. It passes that result to `00BFFB50` and stores returned EAX through saved ESI before returning. The errno-accessor-before-GetLastError order is actual assembly evidence.

The import cells, allocator cells, table and external child bodies were not opened. Import names and semantic names are existing labels, with operand/call order independently checked. No retries, validation, fallback heap, rollback or payload destructor occur in the inspected bodies. Exception/SEH cleanup and child fault behavior remain unproved.

## Existing Source providers and actual domains

`singleton_lifetime_allocate/free` is a genuine matching host-CRT pair: `std::malloc` with `_callnewh` retry and `std::free`. Its existing `00BF65AC` service-boundary admission remains valid. It does not establish that original gunnery nodes, or the still-unopened producer `00956C20`, belong to that host allocation domain.

`NativeGameArrayLifetimeCalls::free_00bf65ac` defaults to `::operator delete`; its name is not an allocator-domain proof. `NativeSceneRegistryStorageBindings::actual_00bf65ac` requires a real current free binding but implements no dispatcher. Neither supplies the gunnery node allocation/lifetime owner. Their sentinel-list layouts also differ from the earlier gunnery count0/head4/tail8 contract.

The existing `free_native_sbh_block_00c11d68` is a concrete SBH child requiring an actual descriptor, payload, canonical state, feature word and already-owned lock 4. Its extra Source references do not implement the native one-argument dispatcher ABI. Canonical SBH initialization likewise requires an already-valid current heap. Mapped canonical pages alone do not initialize a heap, locks, PTD, mode policy or node ownership.

`unlock_native_crt_canonical_00c11b31` is a concrete Source child: its naked body reads the current fixed descriptor at `00E16478 + index*8` and calls real `LeaveCriticalSection`. The caller must own an initialized acquired Win32 critical section. Other critical-section primitives do not supply the unopened index-based `00C11C21` acquisition path.

`LegacyCrtMathRuntime` requires the actual owning CRT errno accessor; it is not an implementation of `00BFFB8B` or the `00BFFB50` mapping. Existing `leave_native_crt_seh4_frame_00c07c45` requires the exact existing compiler frame and FS chain. No Source `00C07C00` prolog/table owner was found by address search.

Eighteen inspected Source-file pins match the integrator's current files after LF normalization. This is a bounded provider comparison; Native child bodies and provider build artifacts were not reopened.

## Smallest concrete next Source candidate

`00BF9E1E` is independently complete and has an existing concrete child. A naked no-argument leaf can preserve `PUSH 4; CALL unlock_native_crt_canonical_00c11b31; POP ECX; RET`, using the actual fixed canonical lock and inheriting its initialized/acquired ownership preconditions. `POP ECX` establishes **ECX=4** on normal return. EAX/EDX/flags are child residual state; the helper has no own frame and no EBP-based access.

This candidate needs Root's registration, normal build, emitted whole-object/provider review, and annotation coordination before admission. It supplies no lock initialization/acquisition, heap, gunnery node owner, dispatcher, destructor, SEH table, faithful startup or gameplay closure. A cleanup/unwind role is suggested by the surrounding compiler state writes, but the unopened table is not an exception-path proof. Nothing is implemented in this readiness packet.

For the held parent/destructor, the next exact missing inputs are the gunnery allocation-domain producer `00956C20`, direct descriptor-lookup entry `00C11D3D`, lock acquisition `00C11C21`, actual mode/heap owner, error services `00BFFB8B`/`00BFFB50`, and prolog/table `00C07C00`/`00E02DF0`. The earlier destructor listing gate at `00955EB0` remains separately held; Root owns that repair and this packet never opens or changes it.

## Validation and limits

This packet changes only the two readiness files: no C++, CMake, tests, builds, probes, ledgers or Ghidra mutations. Complete PE/live bytes, saved/live physical starts, all branch/call/return instructions and bounded existing Source contracts were inspected. New Source admissions, original-ABI admissions, startup credit and gameplay credit are all zero.

The current inherited Root receipt is `reports/cc12_self_refresh_renderer_source_primary_review.json`, SHA-256 `ef66b96cb0d2634728bf04b0bf3b1e6c3ee3d1ed136852fa37fe39a1614b2547`, created `2026-10-09T22:00:59.352422+00:00`. It records 509 selected Source inputs, 4 artifacts, 43 selected Core objects plus 1 application object, 51 positive Core roots, and 3 passing existing checks. These results were pinned, not rerun. Its genuine-SDK smoke is 640x480, 3 ticks, 2 Presents, 1 initial skip, exit 0 in PressStartPoll/state 2, and zero mission frames; it has no visual or gameplay proof. Frozen Source121/507 receipts were not compared with current artifacts.
