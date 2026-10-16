# Checked DWORD copy through the current SDK

Address: 00440C70. Complete ordinary body 00440C70..00440C9F: 48 defined bytes, 18 instructions. Established STL_Copy_00440c70 name is preserved; the C++ label is a descriptive hypothesis.

Original ABI: three stacked DWORDs first,last,destination; incoming ECX/EDX are unconsumed and overwritten, RET0Ch, EAX independently captured output. The new public Source interface is a stdcall function with those three pointer inputs and pointer return.

The body captures last,first,destination, computes wrapped DWORD last-first, exact SAR2 bits and wrapped words*4, then captures destination+bytes before the SDK call. Only a zero SAR result skips the call. Negative nonzero counts still call with their unsigned wrapped bytes; positive byte differences 1..3 can skip. The original JZ uses SAR flags across LEA/PUSH/LEA, not flags from the rounded output. SDK errno is ignored and the captured result is returned on every returning path.

The explicitly authorized boundary is the same direct currentSDK memmove_s(destination,bytes,first,bytes) effect as the published NativeGameClassCleanupCalls::class_memmove_00bf67a7 method. No new callback/provider object, context, header, count bridge, allocation, cleanup, guard, rollback or noexcept adapter is introduced. The signed-positive 00467D90 kernel is not reused. Original BF67A7 CRT/validation/underlying move/feature/fault identity remains unproved.

Owned Native transfer:

| Function | Site | Callee | Coverage |
|---|---|---|---|
| 00440C70 | 00440C92 CALL 00BF67A7 | _memmove_s | complete ordinary within CurrentSDK domain |

Separate bounded read-only producer transfers:

| Function | Site | Callee | Scope |
|---|---|---|---|
| 00441860 | 0044191e CALL 00440C70 | STL_Copy_00440c70 | bounded external producer only |
| 00441860 | 0044193d CALL 00440C70 | STL_Copy_00440c70 | bounded external producer only |
| 00441860 | 004419a0 CALL 00440C70 | STL_Copy_00440c70 | bounded external producer only |
| 00441860 | 004419e8 CALL 00440C70 | STL_Copy_00440c70 | bounded external producer only |

Fresh bsp.gpr/program verification, all 48 owned PE/live bytes and 18 current starts, actual CALL target, four producer CALLs / 20 bytes and their bounded setup windows were checked. All 64 Source/header provider LF fingerprints still match published baseline 2f48232d4d90fd7dc781bff81682b2e8fe9ba2c0 and frozen readiness 6f7555040e0dd05788bdf71bedf9abf3132cf4f5454a2b19ad4cfcd92c404efb.

Default ./scripts/build.ps1 passed with exit0; all three existing suites passed in 7.53s. These suites do not execute this helper. No new tests, probes or Native execution were added.

The actual CMake I386 object was fully reviewed: 191 code bytes / 70 instructions, two CODE sections. Public 97B/37 instructions has two RET0Ch paths; SDKinline/private helper 94B/33 instructions includes its generic ERANGE path. The public equal-size/count call excludes ERANGE. ESI captures result at+25 before any SDK errno/invalid/_memmove call and supplies both returns. The SDKinline byte-zero test is distinct from the outer SAR-result test; neither adds a signed-positive filter.

All eight relocations are accounted for: three errno DIR32, three invalid-parameter DIR32 and two current _memmove REL32. Full private SDKzero/EINVAL/ERANGE/move/return code was read. Noncode metadata is 47B linker directives, 212B compiler debug and 40B checksums. No padding, embedded code data, BSS or EH metadata is present. These are physical pre-link object counts; no runtime execution or linked-size claim is made.

Compiler destination-load reordering, new private frames/argument homes/registers/flags and current SDKerror policy are qualified Source details. Matching RET arity is not Original binary/FH3/SEH/fault/heap/game proof. No OriginalCRT body or whole 00441860/00492210/table-loader/constructor Source is promoted.
