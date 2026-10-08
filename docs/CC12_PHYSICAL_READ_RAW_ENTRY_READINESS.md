# Physical raw read readiness: 00BF5030

Raw read is **not ready for Source authorization or connected native comparison**. The complete failure branch consumes the actual published VFS owner and the native `00BD9E30` callback entry. Current Source provides different interfaces for that operation. Successful Win32 reads do not test the missing contract.

This packet changes only this document and its JSON report. It performs no C++/CMake/ledger/Ghidra edits, compilation, linking, fixture replay, native calls, or game execution. The audit baseline is `3c40ad77f203b5797ece4248b0ae71543be20f67`.

## Complete original read body

Live Ghidra in `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, and the installed PE agree on every byte of `00BF5030-00BF5083`: **84 bytes, 30 instructions**, SHA-256 `d0b1d2fd5e3863f9e82af8f94b038bb84c2fc189c683fd5f3a31fd92397ad48d`. The complete bytes and operand-level disassembly are physically retained under `local/cc12_physical_read_readiness_20261008a/audit01`.

At entry, ECX is the actual backing object. Stack arguments are destination, requested DWORD, and optional output pointer. The object carries HANDLE at +08 and cached position at +10/+14. `PUSH ECX` reserves a local count word at entry ESP-4; it is zeroed before the actual `ReadFile` call through `KERNEL32.dll!ReadFile`, IAT `00CE2208`, with null overlapped input. The requested argument is not reused as count storage.

If ReadFile returns false, the body loads ECX from actual `0109CEEC`, loads EDX from manager+18, pushes that word, and calls `00BD9E30`. Only after the callback returns does it reload the actual count, ADD the low cached position, ADC the high position, and optionally store the count. The intervening MOV preserves carry; the optional output store follows both cache writes and therefore retains alias behavior. Finally POP ECX returns the local count in ECX as well as EAX, and RET0C discards the three stack arguments. ESI is preserved. EDX and flags remain unqualified. There is no retry, null-owner guard, count fallback, size-cache update, or last-error preservation helper.

The exact relocation-bearing operands are the ReadFile IAT at byte35, actual-global address at byte45, and BD9E30 relative call at byte54. These are factual Original operands, not a Source relocation allowance or current implementation.

## First missing whole contract

`00BD9E30-00BD9E3A` is exactly **11 bytes, 3 instructions**: `MOV EAX,[ECX+90h]; CALL EAX; RET4`. It forwards the actual manager in ECX and leaves incoming EDX visible to the callback. From BF5030, EDX contains manager+18. The pushed manager+18 word is discarded by RET4; it is not pushed as a callback argument. The zero-parameter Ghidra prototype does not describe this raw ABI.

Current whole Source/COFF evidence is:

| Provider | Whole bytes / instructions | Established boundary |
| --- | --- | --- |
| Typed physical read BF5030 | 103 / 42 | cdecl plus context; directly calls field90 as a fastcall pointer |
| Typed notify BD9E30 | 24 / 11 | cdecl manager/discarded/dispatch; calls injected virtual dispatch slot+0C |
| Runtime failure dispatcher | 15 / 5 | forwards to NativeVfsRuntimeBindings |
| Runtime open-failure binding | 27 / 9 | accepts numeric identity00530620; invokes Source callback |
| Source callback00530620 | 3 / 1 | complete `RET 0` (`C2 0000`); Original is complete `RET` (`C3`) |
| Source startup callback installation | 32 / 9 | stores original numeric callback identities through supplied publication reference |

The empty callback's no-pop return behavior is understood, but it does not establish the complete owner/dispatch path. The Source notifier's cdecl and virtual interface is not a raw ECX/EDX/RET4 replacement. The typed physical helper and the runtime identity dispatcher also use different callback domains.

## Owner and publication evidence

`0109CEEC` lies in the zero-filled tail of Original `.data`, beyond its 65,536 raw bytes. Its loader-initial DWORD is zero; Ghidra also shows zero. This is not a runtime manager or a preimage of manager+18/+90. No owner bytes were captured or manufactured.

The bounded global-reference inspection identifies a direct publication write at `00BDA746` inside manager-base constructor `00BDA6F0`, and clearing at `00BDA7F9`. The existing Source base constructor publishes a caller-supplied owner through a volatile reference under its lifetime guard, then registers current publication. `NativeVfsOwnerServices` shares the caller-owned VFS publication cell with the physical context and runtime. `GameNativeVfsRuntime::register_core` constructs a derived manager in caller-owned A0h storage, republishes it, and installs startup callbacks. These Source ownership APIs do not establish a live owner at the Original absolute address, and their native producer/lifetime graph is not admitted here.

The exact **32-byte / 4-instruction fragment** at `0073D63C-0073D65B` reloads actual `0109CEEC` before each store: +90=`00530620`, then +8C=`00735B30`. It is not a whole-function claim for Application_Initialize. Source startup likewise writes the original numeric identity into +90. The typed physical read's direct function-pointer call cannot be treated as proof that this production identity-based owner route works.

## Retained evidence and next packet

The immutable audit manifest has SHA-256 `297a2fbb8edd161cec436978e8e3908f6d7e8ec7e7891de0eb140ae2e800fc3c` and **455 physically copied rows**: seven whole Source providers, eight headers, seven whole current COFF objects, the whole current core archive, the whole Original PE, 371 prior compiler-consumed inputs, 46 compiler/linker backend files, two OS DLLs, the kernel32 import library, and evidence/parser/backend inputs. Duplicate roles may retain the same physical source path; 455 is a manifest-row count, not a claim of 455 unique files. Every row's pre/copy/post hash agrees. Every complete provider object occurs exactly once by bytes/hash inside the retained archive. Whole selected-function COFF evidence has SHA-256 `a353524787ce01ab087ca4c7f5c6fba7daca426f42d114f79809fb794e5df57a`.

The archive is the existing normal-build result from the raw-write packet; it was not rebuilt or linked in this audit. Compiler/OS/import-library copies do not imply their execution. The prior raw-write manifest also records unused SDK/header candidates as identity pins; those candidates must not be counted as physical copies. Accepted constructor43 and physical-table findings are reused as prerequisites, with no replay or whole-class admission.

The smallest next child is the native failure notifier `00BD9E30` with the bounded startup callback `00530620`. Primary review must authorize any Source implementation. A genuine published owner and its field18/+90 preimage remain prerequisites before connected failure-path execution. If that producer is unavailable, identify it in a separate bounded owner audit; do not substitute a fake global, default owner, synthetic callback, typed adapter, or success-only test. No whole-VFS expansion is authorized by this audit.

Machine-readable details: [cc12_physical_read_raw_entry_readiness.json](../reports/cc12_physical_read_raw_entry_readiness.json). This packet establishes static readiness limits, not raw ABI compatibility, runtime correctness, or gameplay validation.
