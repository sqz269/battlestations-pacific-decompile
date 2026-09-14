# Actual raw-reader construction: BEA150 and BF09A0

This packet implements the complete `BEA150` construction operation in the established valid, nonfaulting source domain: fresh caller-owned writable `70h` storage and the stable actual `415270` header initializer. It introduces no generic unwind, terminate, allocation, string-pool, or stream substitute. Native FH3/SEH identity, asynchronous faults, replaced callbacks, concurrent mutation, and gameplay behavior remain unclaimed.

The source is [native_raw_reader_construction.cpp](../src/native_raw_reader_construction.cpp), with its interface in [native_raw_reader_construction.hpp](../include/bsp/native_raw_reader_construction.hpp). It builds into `bsp_core` through one appended registration in `cmake/startup.cmake`; the existing provider and `CMakeLists.txt` are unchanged.

## Evidence and original ABI

The reviewed discovery is commit `f57a2fba9bef3751f78c8f7ed062c1680f26c573`, report SHA-256 `301e1e12d6b46a463fdbb15083e6a8dade848f814785ee57ee003418805cc996`. Its 291 local artifacts, report, and document are preserved separately in this packet's ignored local evidence and verified with SHA-256 and SHA-512. The original discovery worktree remains intact.

| Address | Original byte range | Original ABI |
| --- | --- | --- |
| `BEA150` | `[00BEA150,00BEA1B5)`, 101 bytes | ECX = writable caller-owned `70h` reader; EAX = same reader; plain RET |
| `BF09A0` | `[00BF09A0,00BF09B0)`, 16 bytes | ECX = writable raw `10h` base; EAX = same base; ECX becomes zero; plain RET |
| Existing `415270` | `[00415270,00415280)`, 16 bytes | ECX = writable raw `8h` header; EAX = same header; plain RET |

These three complete spans were freshly compared between the installed PE and live Ghidra for this packet. The PE SHA-256 is `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`. Each Ghidra CLI call verifies project `bsp`, program `/battlestationspacific.exe`, language and image base against the configuration naming `C:/Users/sqz269/bsp.gpr`. No Ghidra annotations or listing repairs are applied by this worker.

## Exact storage schedule

`construct_native_raw_stream_reader_base_00bf09a0` is the concrete naked MSVC Win32 primitive: copy ECX to EAX, zero ECX, then write zero DWORDs at base offsets `+0,+4,+8,+C` before plain RET. Its assembly preserves individual store order and native unaligned access without requiring aligned C++ DWORD objects.

`construct_native_raw_reader_path_storage_00bea150` calls that primitive once, then directly calls the existing `initialize_native_string_header_00415270` ten times, passing `reader+10h+8*i`. The existing provider in `src/native_renderer_worker_lifetime.cpp` is naked assembly that writes header length zero then data null. The packet neither copies that implementation nor routes through the unrelated generic header-array helper in that file.

After all ten calls, four explicit native DWORD stores write `reader+60h=0`, `reader+64h=FFFFFFFFh`, `reader+68h=0`, and `reader+6Ch=0`, in that order. The constructor returns the original reader pointer. Base `[0,10h)`, ten headers `[10h,60h)`, and tail `[60h,70h)` are disjoint. No reader pointer is published to a global or another object. A previously live reader is not destroyed; this is construction storage, not a reset operation.

The public `BEA150` source entry has a new ordinary C++ call interface. Its source-domain behavior does not claim the original constructor's native exception-frame ABI. The `BF09A0` primitive deliberately retains its original ECX/EAX and plain-RET register schedule. Descriptive function names remain reconstruction hypotheses.

## Why no generic exception provider is needed

The complete native `BF09A0` and `415270` bodies contain no calls, allocations, external provider reads, or C++ throw operation. The concrete existing source header provider is exactly those four native instructions. Their writes cannot throw a C++ exception in valid, nonfaulting storage. `BEA150` fixes its constructor callback to `415270`; it does not accept arbitrary callbacks.

Consequently completed-prefix cleanup and outer base cleanup are unreachable in this source domain. Direct construction closes this actual operation without implementing the CRT's arbitrary throwing-callback vector iterator. No string-pool context, destructor callback, catch-all, or terminate callback is introduced merely to satisfy an unresolved generic signature.

The discovery preserves the separate native exceptional contracts. `BF7D1E` uses its parent EBP to unwind only completed headers in reverse order. `BF7C33` returns zero for non-C++ exception codes and calls the existing native terminate routine for `E06D7363`. The outer `C07991` handler also has an actual TLS/exception-code/FuncInfo eligibility gate, with EHFlags one for `BEA150`; unconditional outer catch-all cleanup is not equivalent. None of those native fault or runtime-provider claims follows from this source specialization. Root-node allocation, stream attachment, and general reader destruction remain outside the packet.

## Validation and retained artifacts

The strict MSVC Win32 Release build passed, all eight PE/live seed comparisons matched, and both existing CTests passed. The accompanying [report](../reports/native_raw_reader_construction_bn.json) retains the exact constructor object, existing provider object, current `bsp_core.lib`, unique member listing and extracted-member equality, compiler tlogs, generated project/cache, CTest log, and source/native provenance.

The emitted new constructor is 71 bytes. Its only two call sites are the concrete base primitive and the existing header provider; the latter runs in the verified zero-to-ten loop. Final stores occur after that loop, followed by returning the saved reader pointer. Both the base and existing header primitives are exactly 16 emitted bytes matching their original PE/live spans. Each object occurs exactly once in the current archive, and its extracted member equals the captured object. These are compiled-artifact checks, not constructor execution.

Two independent SHA-256/SHA-512 inventories cover the same frozen local artifact set; the final report also hashes the inventory files themselves. No new tests, executable probes, native runtime comparisons, or game tests are added or run. The existing CTests cover their established math behavior and are not constructor runtime validation.
