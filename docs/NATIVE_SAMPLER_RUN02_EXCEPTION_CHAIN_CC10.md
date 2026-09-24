# Run02 invalid-handle call chain

The retained run02 x86 context and stack identify a concrete `CloseHandle(00400000)` path whose caller return lies inside the copied `xlive.dll`. Matching disk bodies corroborate both exact CALL return boundaries. This narrows the obstruction in `NATIVE_SAMPLER_ORIGINAL_CAPTURE_RUN02_CC10.md`; it does not recover the sampler input or establish a successful continuation.

| Captured value | Interpretation supported by matching code |
| --- | --- |
| EIP77879C7C | NtClose+0C, RET4 |
| ESP0162F420 contains75B1E1D7; next DWORD00400000 | CloseHandle+57 return from imported NtClose; current argument |
| EBP0162F434 contains0162F548; next DWORD6AE1C270; argument00400000 | Saved xlive EBP, return at xlive+17C270 immediately after its CloseHandle CALL, same argument |
| Caller EBP-2C contains00E2F000; EBP-38 contains2 | PE image-size local and VirtualProtect old-protection local |

Both exception records have identical716-byte WOW64 contexts and320-byte WOW64 stack windows. The xlive172B body at RVA17C1F0..17C29C calls GetModuleHandleA(NULL), stores its result, reads that module's PE SizeOfImage, calls VirtualProtect with protection40h, then passes the saved module to CloseHandle. Its intervening stack-check thunk preserves EAX. The frame allocation and saved registers reproduce the captured frame locations exactly. The next xlive return is outside the captured window. Live xlive instruction bytes and IAT values were not captured; imported API identities and this body come from the hash-matching copied file.

Microsoft documents that GetModuleHandleA(NULL) returns the calling executable's module handle, and invalid/pseudo handles passed to CloseHandle raise an exception under a debugger. These contracts support the observed API path. They do not prove what resuming this particular event would return. [GetModuleHandleA](https://learn.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-getmodulehandlea), [CloseHandle](https://learn.microsoft.com/en-us/windows/win32/api/handleapi/nf-handleapi-closehandle).

The xlive VirtualProtect call is a supported explanation for initial entry protection20h versus cleanup-observed40h. The captured size matches the executable's00E2F000 SizeOfImage. Old-protection local2 refers to the module's first page, not its executable entry page. No API call/result or writer trace was captured, so this remains an inference.

The first native context is at ntdll KiRaiseUserExceptionDispatcher+3A, RIP7FF9CFC64EBA/RSPAE610. Its exact unwind entry describes a C8-byte allocation. Captured [RSP+C0] is C0000008; [RSP+C8] is777F1F42, the exact wow64cpu return after its syscall-thunk CALL. The dispatcher continuation reloads EAX from that saved status, restores RSP and returns. KernelBase's matching CloseHandle body tests the returned status and has a negative-status BOOL0 path. Actual resumed BOOL/GetLastError remains unobserved.

The second raw exception addressCFC64EBA is MEM_FREE and is distinct from its native RIP7FF9CF5E7401 in wow64.dll. The matching wrapper/helper uses RtlCaptureContext, lookup/unwind and NtRaiseException. A bounded wrapper unwind reaches the exact caller return6EA1; the next required slot lies outside the captured window. The matching caller's32-to64-bit record conversion zero-extends its32-bit address field. This is consistent with the observed low32 record address, but the input record and handler trace were not captured; it does not prove corrupt execution at that address.

All five nonempty live code windows match the matching disk bytes after relocation. The x86 windows differ from raw disk only in relocated immediates of neighboring syscall stubs. The primary independently decoded the raw contexts, concrete frame slots/arguments, saved native status and unwind encoding, and verified all59 indexed artifacts. The immutable archive and review receipt are indexed in `reports/native_sampler_run02_exception_chain_cc10.json`.

This audit launched no process and changed no game, profile, helper, policy, overlay or Ghidra state. A separate benign fixture will compare the actual pre-entry API call with and without debugging before any original-specific continuation policy is considered. No future compiler frame exists in these pre-entry windows, so the sampler's source0 word remains unknown. Complete native unwind, full XSTATE, successful original startup and gameplay remain unclaimed.
