# Native raw-reader retained attachment

`assign_native_raw_reader_stream_00bf0430` reconstructs the complete 57-byte `BF0430..BF0468` operation in the established raw memory and physical stream provider domains. It publishes and retains the replacement before releasing the captured old stream. If the selected old terminal throws, publication and retention remain in effect; the assignment has no rollback or later reader store.

The [source](../src/native_raw_reader_attachment.cpp) and [header](../include/bsp/native_raw_reader_attachment.hpp) use the existing `NativeRawScalarReaderContext`. This is a context-bearing C++ cdecl interface returning `void`. The original ABI passes the reader in ECX and the stream on the stack, ends with `RET 4`, and has no stable semantic EAX return. Native caller-frame, register ABI, SEH/FH3, asynchronous-fault and gameplay identity are not claimed.

## Evidence and ordered operation

The source packet starts at `0b3bbf7e21a202d2a31dcaba3e6c2631737b415b` and retains discovery `36195e5c03b8240cd5d21229b01dd0a68fe63fbe`. All 473 discovery artifacts, its report and its document are copied with SHA-256 and SHA-512 verification; the original discovery and constructor worktrees are preserved. The discovery contains 24 complete PE/live spans, 37 call rows, actual provider profiles, direct terminal bodies and the separately qualified destructor/storage frontier.

The new packet also rechecks the full BF0430 span against the installed PE and live Ghidra. The PE SHA-256 is `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`. Live CLI queries verify project `bsp`, `/battlestationspacific.exe` and the configuration naming `C:/Users/sqz269/bsp.gpr`. BF0430 has no physical listing gap. Its indirect imports are actual `InterlockedIncrement` at `CE221C` and `InterlockedDecrement` at `CE2220`; its final indirect call is current old profile slot zero.

The source preserves this order:

1. Capture the incoming pointer, then capture `reader+0` as the old pointer.
2. Identical pointers return without any store, atomic operation, profile read or provider/context access.
3. Publish the replacement to `reader+0` (`BF043D`).
4. For nonnull replacement, apply the real Windows `InterlockedIncrement` to replacement `+4` (`BF0445`).
5. For nonnull captured old, apply `InterlockedDecrement` to captured old `+4` (`BF0453`). Only its zero result reads the current old profile and slot zero (`BF045D/BF045F`).
6. Invoke the selected established terminal (`BF0463`). No assignment code writes the reader afterward.

The raw DWORD loads and publication use the same MSVC Win32 assembly-access convention as existing providers. They preserve accesses to actual raw storage and alias-visible changes without imposing a typed host object layout. Counts obey the actual Win32 atomic-storage contract. A new stream retains its reference independently of any reference held by the caller; null detaches. No direct reader assignments at `+4..+6F`, seek, path/header reset, allocation, or reader destruction are added. Aliased count locations and actual provider effects can still change memory there.

## Concrete terminal dispatch

Original table words are evidence-qualified dispatch identities, not callable host addresses. Source dispatch checks only the selected current terminal path. Unsupported current profiles/slots raise a source-domain `std::invalid_argument`, following the existing raw scalar reader policy; that exception is not a recovered native error.

| Selected current profile and slot zero | Existing source provider | Required current lookup and ownership |
| --- | --- | --- |
| Memory `D642C0 / BD30E0` | `delete_native_memory_stream_00bb8f90(old, 1, *context.memory)` | Read current slot zero through the actual memory table; reload the old object's profile as BD30E0 does; read its current slot four and require `BB8F90`; pass flags one. |
| Physical `D691B0 / BF55A0` | `recycle_native_physical_stream_00bf55a0(old, *context.physical)` | Read current slot zero at actual numeric D691B0; let the existing recycle provider own its current slot-four lookup, flags-zero destruction, real pool getter and append. |

The memory context borrows the actual `D642C0` table and existing counters/providers. The terminal retains the existing backing-release and scalar-free schedule. The second current-profile and table read is not replaced by a cached dispatch decision. Physical D691B0 must be readable at its actual numeric address, as required by the existing physical provider. Recycling preserves storage for the real pool; memory flags-one deletion is not substituted for it. Neither branch decrements the old reference a second time.

Only the selected zero-terminal branch requires its nonnull context pointer. Same-pointer, null-old and nonzero-old-reference paths do not dereference either service. The attachment function owns no automatic cleanup and catches no exception. A throwing actual terminal or physical pool operation leaves the replacement already published and retained. Existing terminal cleanup retains its own source contract; this packet adds none.

The valid source domain requires writable actual reader storage, real intrusive stream counts, supported current terminal profiles/slots and stable actual providers. Arbitrary profiles, hooked or concurrently changing provider identities, native operating-system/CRT identity and native fault/frame behavior remain outside this claim. The accepted BEA150 constructor's fresh, nonfaulting caller-owned `70h` contract is unchanged.

## Build and scope

The accompanying [report](../reports/native_raw_reader_attachment_bp.json) records the strict Release MSVC Win32 build, eight matching seed spans and the two existing CTests, along with current compiler commands, object code and relocation review. No new test, attachment executable, native fault probe or game run was added. Existing math tests do not establish attachment runtime behavior.

The emitted attachment is 323 bytes. It loads incoming at `+06` before old at `+0F`, branches on identity at `+1A`, and publishes at `+26`. MSVC lowers the Windows Interlocked expressions to `LOCK INC [new+4]` at `+2F` and `LOCK XADD [old+4], -1` at `+44`; `JNE +48` uses the resulting decrement's zero flag. **These are inline atomic operations, not the native CE221C/CE2220 import calls.** Normal atomic/count-zero source behavior is preserved within the declared Win32 storage domain; original import-call boundaries, hooking and native fault/register effects are not claimed. Real memory and physical terminal calls have REL32 relocations at `+DB` and `+115`. The memory path retains the second profile load at `+97` and current slot-four read at `+BD`. No attachment FS registration or later direct reader write is emitted.

Current attachment, retained-memory and physical-stream object files are retained alongside the exact current `bsp_core.lib`, unique archive-member listings and byte-identical extracted members. COFF evidence identifies the emitted atomic operations, ordered accesses and direct real-provider bindings. Full compiler tlogs, source/header hashes and two matching whole-local SHA-256/SHA-512 inventories are retained; the final report includes both manifests.

The only integration changes are one deferred `bsp_core` source registration and the owned BF0430 reconstruction record. Existing providers, main CMake body, tests, game installation and Ghidra analysis are unchanged. General BE9F10/BF09B0 destruction, BF0700/BF05D0 storage, generic array unwind and the root factory remain the explicit discovery frontier documented in BO.
